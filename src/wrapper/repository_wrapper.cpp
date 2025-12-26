#include <iostream>

#include "../utils/git_exception.hpp"
#include "../wrapper/index_wrapper.hpp"
#include "../wrapper/object_wrapper.hpp"
#include "../wrapper/commit_wrapper.hpp"
#include "../wrapper/remote_wrapper.hpp"
#include <git2/repository.h>
#include <git2/remote.h>
#include "../wrapper/repository_wrapper.hpp"

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

    git_tree* tree;
    index_wrapper index = this->make_index();
    git_oid tree_id = index.write_tree();
    index.write();

    throw_if_error(git_tree_lookup(&tree, *this, &tree_id));

    throw_if_error(git_commit_create(&commit_id, *this, update_ref.c_str(), author_committer_signatures.first, author_committer_signatures.second,
        message_encoding, message.data(), tree, parents_count, parents));

    git_tree_free(tree);
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

// Trees

void repository_wrapper::checkout_tree(const object_wrapper& target, const git_checkout_options opts)
{
    throw_if_error(git_checkout_tree(*this, target, &opts));
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
