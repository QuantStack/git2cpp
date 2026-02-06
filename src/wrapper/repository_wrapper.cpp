#include <algorithm>
#include <fstream>
#include <iostream>

#include "../utils/git_exception.hpp"
#include "../wrapper/index_wrapper.hpp"
#include "../wrapper/object_wrapper.hpp"
#include "../wrapper/commit_wrapper.hpp"
#include "../wrapper/remote_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"
#include "config_wrapper.hpp"
#include "diff_wrapper.hpp"

repository_wrapper::~repository_wrapper()
{
    git_repository_free(p_resource);
    p_resource=nullptr;
}

repository_wrapper repository_wrapper::open(std::string_view directory)
{
    repository_wrapper rw;
    throw_if_error(git_repository_open(&(rw.p_resource), directory.data()));
    return rw;
}

repository_wrapper repository_wrapper::init(std::string_view directory, bool bare)
{
    repository_wrapper rw;
    throw_if_error(git_repository_init(&(rw.p_resource), directory.data(), bare));
    return rw;
}

repository_wrapper repository_wrapper::clone(std::string_view url, std::string_view path, const git_clone_options& opts)
{
    repository_wrapper rw;
    throw_if_error(git_clone(&(rw.p_resource), url.data(), path.data(), &opts));
    return rw;
}

std::string repository_wrapper::git_path() const
{
    return git_repository_path(*this);
}

git_repository_state_t repository_wrapper::state() const
{
    return git_repository_state_t(git_repository_state(*this));
}

void repository_wrapper::state_cleanup()
{
    throw_if_error(git_repository_state_cleanup(*this));
}

bool repository_wrapper::is_bare() const
{
    return git_repository_is_bare(*this);
}

bool repository_wrapper::is_shallow() const
{
    return git_repository_is_shallow(*this);
}

revwalk_wrapper repository_wrapper::new_walker()
{
    git_revwalk* walker;
    throw_if_error(git_revwalk_new(&walker, *this));
    return revwalk_wrapper(walker);
}

// Head

bool repository_wrapper::is_head_unborn() const
{
    return git_repository_head_unborn(*this) == 1;
}

reference_wrapper repository_wrapper::head() const
{
    git_reference* ref;
    throw_if_error(git_repository_head(&ref, *this));
    return reference_wrapper(ref);
}

std::string repository_wrapper::head_short_name() const
{
    git_reference* ref;
    std::string name;
    throw_if_error(git_reference_lookup(&ref, *this, "HEAD"));
    if (git_reference_type(ref) == GIT_REFERENCE_DIRECT)
    {
         name = git_reference_shorthand(ref);
    }
    else
    {
        name = git_reference_symbolic_target(ref);
        name = name.substr(name.find_last_of('/') + 1);
    }
    git_reference_free(ref);
    return name;
}

// References

reference_wrapper repository_wrapper::find_reference(std::string_view ref_name) const
{
    git_reference* ref;
    throw_if_error(git_reference_lookup(&ref, *this, ref_name.data()));
    return reference_wrapper(ref);
}

std::optional<reference_wrapper> repository_wrapper::find_reference_dwim(std::string_view ref_name) const
{
    git_reference* ref;
    int rc = git_reference_dwim(&ref, *this, ref_name.data());
    return rc == 0 ? std::make_optional(reference_wrapper(ref)) : std::nullopt;
}

// Index

index_wrapper repository_wrapper::make_index()
{
    index_wrapper index = index_wrapper::init(*this);
    return index;
}

// Branches

branch_wrapper repository_wrapper::create_branch(std::string_view name, bool force)
{
    return create_branch(name, find_commit(), force);
}

branch_wrapper repository_wrapper::create_branch(std::string_view name, const commit_wrapper& commit, bool force)
{
    git_reference* branch = nullptr;
    throw_if_error(git_branch_create(&branch, *this, name.data(), commit, force));
    return branch_wrapper(branch);
}

branch_wrapper repository_wrapper::create_branch(std::string_view name, const annotated_commit_wrapper& commit, bool force)
{
    git_reference* branch = nullptr;
    throw_if_error(git_branch_create_from_annotated(&branch, *this, name.data(), commit, force));
    return branch_wrapper(branch);
}

branch_wrapper repository_wrapper::find_branch(std::string_view name) const
{
    git_reference* branch = nullptr;
    throw_if_error(git_branch_lookup(&branch, *this, name.data(), GIT_BRANCH_LOCAL));
    return branch_wrapper(branch);
}

branch_iterator repository_wrapper::iterate_branches(git_branch_t type) const
{
    git_branch_iterator* iter = nullptr;
    throw_if_error(git_branch_iterator_new(&iter, *this, type));
    return branch_iterator(iter);
}

// Commits

commit_wrapper repository_wrapper::find_commit(std::string_view ref_name) const
{
    git_oid oid_parent_commit;
    throw_if_error(git_reference_name_to_id(&oid_parent_commit, *this, ref_name.data()));
    return find_commit(oid_parent_commit);
}

commit_wrapper repository_wrapper::find_commit(const git_oid& id) const
{
    git_commit* commit;
    throw_if_error(git_commit_lookup(&commit, *this, &id));
    return commit_wrapper(commit);
}

void repository_wrapper::create_commit(const signature_wrapper::author_committer_signatures& author_committer_signatures,
    const std::string_view message, const std::optional<commit_list_wrapper>& parents_list)
{
    const char* message_encoding = "UTF-8";
    git_oid commit_id;

    std::string update_ref = "HEAD";
    const git_commit* placeholder[1] = {nullptr};

    auto [parents, parents_count] = [&]() -> std::pair<const git_commit**, size_t>
    {
        if (parents_list)
        {
            // TODO: write a "as_const" function to replace the following
            auto pl_size = parents_list.value().size();
            git_commit** pl_value = parents_list.value();
            auto pl_value_const = const_cast<const git_commit**>(pl_value);
            return {pl_value_const, pl_size};
        }
        else
        {
            auto parent = revparse_single(update_ref);
            size_t parents_count = 0;
            if (parent)
            {
                parents_count = 1;
                placeholder[0] = *parent;
            }
            return {placeholder, parents_count};
        }
    }();

    index_wrapper index = this->make_index();
    git_oid tree_id = index.write_tree();
    index.write();

    auto tree = this->tree_lookup(&tree_id);

    throw_if_error(git_commit_create(&commit_id, *this, update_ref.c_str(), author_committer_signatures.first, author_committer_signatures.second,
        message_encoding, message.data(), tree, parents_count, parents));
}

std::optional<annotated_commit_wrapper> repository_wrapper::resolve_local_ref
(
    const std::string_view target_name
) const
{
    if (auto ref = this->find_reference_dwim(target_name))
    {
        return this->find_annotated_commit(*ref);
    }
    else if (auto obj = this->revparse_single(target_name))
    {
        return this->find_annotated_commit(obj->oid());
    }
    else
    {
        return std::nullopt;
    }
}

// Annotated commits

annotated_commit_wrapper repository_wrapper::find_annotated_commit(const git_oid& id) const
{
    git_annotated_commit* commit;
    throw_if_error(git_annotated_commit_lookup(&commit, *this, &id));
    return annotated_commit_wrapper(commit);
}

// Objects

std::optional<object_wrapper> repository_wrapper::revparse_single(std::string_view spec) const
{
    git_object* obj;
    int rc = git_revparse_single(&obj, *this, spec.data());
    return rc == 0 ? std::make_optional(object_wrapper(obj)) : std::nullopt;
}

object_wrapper repository_wrapper::find_object(const git_oid id, git_object_t type)
{
    git_object* object;
    git_object_lookup(&object, *this, &id, type);
    return object_wrapper(object);
}

// Head manipulations

void repository_wrapper::set_head(std::string_view ref_name)
{
    throw_if_error(git_repository_set_head(*this, ref_name.data()));
}

void repository_wrapper::set_head_detached(const annotated_commit_wrapper& commit)
{
    throw_if_error(git_repository_set_head_detached_from_annotated(*this, commit));
}

void repository_wrapper::reset(const object_wrapper& target, git_reset_t reset_type, const git_checkout_options& checkout_options)
{
    // TODO: gerer l'index

    throw_if_error(git_reset(*this, target, reset_type, &checkout_options));
}

size_t repository_wrapper::shallow_depth_from_head() const
{
    if (!this->is_shallow())
    {
        return 0u;
    }

    std::string git_path = this->git_path();
    std::string shallow_path = git_path + "shallow";

    std::vector<git_oid> boundaries_list;
    std::ifstream f(shallow_path);
    std::string line;
    while (std::getline(f, line))
    {
        if (!line.empty())
        {
            git_oid commit_oid;
            git_oid_fromstrp(&commit_oid, line.c_str());
            boundaries_list.push_back(commit_oid);
        }
    }

    if (boundaries_list.size() == 0u)
    {
        return 0u;
    }

    commit_wrapper head_commit = this->find_commit("HEAD");
    commit_list_wrapper commits_list = head_commit.get_parents_list();
    std::vector<size_t> depth_list(commits_list.size(), 1u);
    std::vector<size_t> final_depths(boundaries_list.size(), 1u);
    bool has_parent = commits_list.size() > 0u;
    while (has_parent)
    {
        has_parent = false;
        std::vector<commit_wrapper> temp_commits_list;
        std::vector<size_t> temp_depth_list;
        commit_list_wrapper parent_list({});

        for (size_t i = 0u; i < commits_list.size(); i++)
        {
            const commit_wrapper& commit = commits_list[i];
            size_t depth = depth_list[i];
            const git_oid& oid = commit.oid();
            bool is_boundary = std::find_if(boundaries_list.cbegin(), boundaries_list.cend(), [oid](const git_oid& val) {return git_oid_equal(&oid, &val);}) != boundaries_list.cend();
            if (is_boundary)
            {
                final_depths.push_back(depth + 1u);
            }
            else
            {
                parent_list = commit.get_parents_list();
                if (parent_list.size() > 0u)
                {
                    has_parent = true;
                    for (size_t j = 0u; parent_list.size(); j++)
                    {
                        const commit_wrapper& c = parent_list[j];
                        temp_commits_list.push_back(std::move(const_cast<commit_wrapper&>(c)));
                        temp_depth_list.push_back(depth + 1u);
                    }
                }
            }
        }
        depth_list = temp_depth_list;
        commits_list = commit_list_wrapper(std::move(temp_commits_list));
    }

    std::size_t depth = *std::max_element(final_depths.begin(), final_depths.end());
    return depth;
}

// Trees

void repository_wrapper::checkout_tree(const object_wrapper& target, const git_checkout_options opts)
{
    throw_if_error(git_checkout_tree(*this, target, &opts));
}

tree_wrapper repository_wrapper::tree_lookup(const git_oid* tree_id)
{
    git_tree* tree;
    throw_if_error(git_tree_lookup(&tree, *this, tree_id));
    return tree_wrapper(tree);
}

tree_wrapper repository_wrapper::treeish_to_tree(const std::string& treeish)
{
    auto obj = this->revparse_single(treeish.c_str());
    git_tree* tree = nullptr;
    throw_if_error(git_object_peel(reinterpret_cast<git_object**>(&tree), obj.value(), GIT_OBJECT_TREE));
    return tree_wrapper(tree);
}

// Remotes

remote_wrapper repository_wrapper::find_remote(std::string_view name) const
{
    git_remote* remote = nullptr;
    throw_if_error(git_remote_lookup(&remote, *this, name.data()));
    return remote_wrapper(remote);
}

remote_wrapper repository_wrapper::create_remote(std::string_view name, std::string_view url)
{
    git_remote* remote = nullptr;
    throw_if_error(git_remote_create(&remote, *this, name.data(), url.data()));
    return remote_wrapper(remote);
}

void repository_wrapper::delete_remote(std::string_view name)
{
    throw_if_error(git_remote_delete(*this, name.data()));
}

void repository_wrapper::rename_remote(std::string_view old_name, std::string_view new_name)
{
    git_strarray problems = {0};
    int error = git_remote_rename(&problems, *this, old_name.data(), new_name.data());
    if (error != 0)
    {
        for (size_t i = 0; i < problems.count; ++i)
        {
            std::cerr << problems.strings[i] << std::endl;
        }
        git_strarray_dispose(&problems);
        throw_if_error(error);
    }
    git_strarray_dispose(&problems);
}

void repository_wrapper::set_remote_url(std::string_view name, std::string_view url, bool push)
{
    if (push)
    {
        throw_if_error(git_remote_set_pushurl(*this, name.data(), url.data()));
    }
    else
    {
        throw_if_error(git_remote_set_url(*this, name.data(), url.data()));
    }
}

std::vector<std::string> repository_wrapper::list_remotes() const
{
    git_strarray remotes = {0};
    throw_if_error(git_remote_list(&remotes, *this));

    std::vector<std::string> result;
    for (size_t i = 0; i < remotes.count; ++i)
    {
        result.emplace_back(remotes.strings[i]);
    }

    git_strarray_dispose(&remotes);
    return result;
}


// Config

config_wrapper repository_wrapper::get_config()
{
    git_config* cfg;
    throw_if_error(git_repository_config(&cfg, *this));
    return config_wrapper(cfg);
}


// Diff

diff_wrapper repository_wrapper::diff_tree_to_index(tree_wrapper old_tree, std::optional<index_wrapper> index, git_diff_options* diffopts)
{
    git_diff* diff;
    git_index* idx = nullptr;
    if (index)
    {
        idx = *index;
    }
    throw_if_error(git_diff_tree_to_index(&diff, *this, old_tree, idx, diffopts));
    return diff_wrapper(diff);
}

diff_wrapper repository_wrapper::diff_tree_to_tree(tree_wrapper old_tree, tree_wrapper new_tree, git_diff_options* diffopts)
{
    git_diff* diff;
    throw_if_error(git_diff_tree_to_tree(&diff, *this, old_tree, new_tree, diffopts));
    return diff_wrapper(diff);
}

diff_wrapper repository_wrapper::diff_tree_to_workdir(tree_wrapper old_tree, git_diff_options* diffopts)
{
    git_diff* diff;
    throw_if_error(git_diff_tree_to_workdir(&diff, *this, old_tree, diffopts));
    return diff_wrapper(diff);
}

diff_wrapper repository_wrapper::diff_tree_to_workdir_with_index(tree_wrapper old_tree, git_diff_options* diffopts)
{
    git_diff* diff;
    throw_if_error(git_diff_tree_to_workdir_with_index(&diff, *this, old_tree, diffopts));
    return diff_wrapper(diff);
}

diff_wrapper repository_wrapper::diff_index_to_workdir(std::optional<index_wrapper> index, git_diff_options* diffopts)
{
    git_diff* diff;
    git_index* idx = nullptr;
    if (index)
    {
        idx = *index;
    }
    throw_if_error(git_diff_index_to_workdir(&diff, *this, idx, diffopts));
    return diff_wrapper(diff);
}
