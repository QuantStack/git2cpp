#pragma once

#include <concepts>
#include <optional>
#include <string_view>

#include <git2.h>

#include "../utils/common.hpp"
#include "../utils/git_exception.hpp"
#include "../wrapper/annotated_commit_wrapper.hpp"
#include "../wrapper/branch_wrapper.hpp"
#include "../wrapper/commit_wrapper.hpp"
#include "../wrapper/config_wrapper.hpp"
#include "../wrapper/diff_wrapper.hpp"
#include "../wrapper/index_wrapper.hpp"
#include "../wrapper/object_wrapper.hpp"
#include "../wrapper/refs_wrapper.hpp"
#include "../wrapper/remote_wrapper.hpp"
#include "../wrapper/revwalk_wrapper.hpp"
#include "../wrapper/signature_wrapper.hpp"
#include "../wrapper/tree_wrapper.hpp"
#include "../wrapper/wrapper_base.hpp"

struct branch_tracking_info
{
    bool has_upstream;
    std::string upstream_name;
    size_t ahead;
    size_t behind;
};

class repository_wrapper : public wrapper_base<git_repository>
{
public:

    ~repository_wrapper();

    repository_wrapper(repository_wrapper&&) noexcept = default;
    repository_wrapper& operator=(repository_wrapper&&) noexcept = default;

    static repository_wrapper init(std::string_view directory, bool bare);
    static repository_wrapper open(std::string_view directory);
    static repository_wrapper clone(std::string_view url, std::string_view path, const git_clone_options& opts);

    std::string git_path() const;
    git_repository_state_t state() const;
    void state_cleanup();

    bool is_bare() const;
    bool is_shallow() const;

    revwalk_wrapper new_walker();

    bool does_track(std::string_view path) const;

    // Head
    bool is_head_unborn() const;
    reference_wrapper head() const;
    std::string head_short_name() const;

    // References
    reference_wrapper find_reference(std::string_view ref_name) const;
    std::optional<reference_wrapper> find_reference_dwim(std::string_view ref_name) const;

    // Index
    index_wrapper make_index();

    // Branches
    branch_wrapper create_branch(std::string_view name, bool force);
    branch_wrapper create_branch(std::string_view name, const commit_wrapper& commit, bool force);
    branch_wrapper create_branch(std::string_view name, const annotated_commit_wrapper& commit, bool force);
    branch_wrapper find_branch(std::string_view name) const;
    branch_iterator iterate_branches(git_branch_t type) const;
    std::optional<reference_wrapper> upstream() const;
    branch_tracking_info get_tracking_info() const;

    // Commits
    commit_wrapper find_commit(std::string_view ref_name = "HEAD") const;
    commit_wrapper find_commit(const git_oid& id) const;
    void create_commit(const signature_wrapper::author_committer_signatures&, const std::string_view, const std::optional<commit_list_wrapper>& parents_list);
    std::optional<annotated_commit_wrapper> resolve_local_ref(const std::string_view target_name) const;

    // Annotated commits
    annotated_commit_wrapper find_annotated_commit(const git_oid& id) const;

    template <std::convertible_to<git_reference*> T>
    annotated_commit_wrapper find_annotated_commit(const T& wrapper) const;

    // Objects
    std::optional<object_wrapper> revparse_single(std::string_view spec) const;
    object_wrapper find_object(const git_oid id, git_object_t type);

    // Head manipulations
    void set_head(std::string_view ref_name);
    void set_head_detached(const annotated_commit_wrapper& commit);
    void reset(const object_wrapper& target, git_reset_t reset_type, const git_checkout_options& checkout_options);

    // TODO: check if it's the right place to put the following
    size_t shallow_depth_from_head() const;

    // Trees
    void checkout_tree(const object_wrapper& target, const git_checkout_options opts);
    tree_wrapper tree_lookup(const git_oid* tree_id);
    tree_wrapper treeish_to_tree(const std::string& treeish);

    // Remotes
    remote_wrapper find_remote(std::string_view name) const;
    remote_wrapper create_remote(std::string_view name, std::string_view url);
    void delete_remote(std::string_view name);
    void rename_remote(std::string_view old_name, std::string_view new_name);
    void set_remote_url(std::string_view name, std::string_view url, bool push = false);
    std::vector<std::string> list_remotes() const;

    // Config
    config_wrapper get_config();

    // Diff
    diff_wrapper diff_tree_to_index(tree_wrapper old_tree, std::optional<index_wrapper> index, git_diff_options* diffopts);
    diff_wrapper diff_tree_to_tree(tree_wrapper old_tree, tree_wrapper new_tree, git_diff_options* diffopts);
    diff_wrapper diff_tree_to_workdir(tree_wrapper old_tree, git_diff_options* diffopts);
    diff_wrapper diff_tree_to_workdir_with_index(tree_wrapper old_tree, git_diff_options* diffopts);
    diff_wrapper diff_index_to_workdir(std::optional<index_wrapper> index, git_diff_options* diffopts);

    //Tags
    // git_strarray_wrapper tag_list_match(std::string pattern);
    std::vector<std::string> tag_list_match(std::string pattern);

private:

    repository_wrapper() = default;
};

template <std::convertible_to<git_reference*> T>
annotated_commit_wrapper repository_wrapper::find_annotated_commit(const T& wrapper) const
{
    git_annotated_commit* commit;
    throw_if_error(git_annotated_commit_from_ref(&commit, *this, wrapper));
    return annotated_commit_wrapper(commit);
}
