#pragma once

#include <concepts>
#include <optional>
#include <string_view>

#include <git2.h>

#include "../utils/git_exception.hpp"
#include "../wrapper/annotated_commit_wrapper.hpp"
#include "../wrapper/branch_wrapper.hpp"
#include "../wrapper/commit_wrapper.hpp"
#include "../wrapper/index_wrapper.hpp"
#include "../wrapper/object_wrapper.hpp"
#include "../wrapper/refs_wrapper.hpp"
#include "../wrapper/wrapper_base.hpp"

class repository_wrapper : public wrapper_base<git_repository>
{
public:

    ~repository_wrapper();

    repository_wrapper(repository_wrapper&&) noexcept = default;
    repository_wrapper& operator=(repository_wrapper&&) noexcept = default;

    static repository_wrapper init(std::string_view directory, bool bare);
    static repository_wrapper open(std::string_view directory);

    git_repository_state_t state() const;

    // References
    reference_wrapper head() const;
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

    // Commits
    commit_wrapper find_commit(std::string_view ref_name = "HEAD") const;
    commit_wrapper find_commit(const git_oid& id) const;

    // Annotated commits
    annotated_commit_wrapper find_annotated_commit(const git_oid& id) const;

    template <std::convertible_to<git_reference*> T>
    annotated_commit_wrapper find_annotated_commit(const T& wrapper) const;

    // Objects
    std::optional<object_wrapper> revparse_single(std::string_view spec) const;

    // Set head
    void set_head(std::string_view ref_name);
    void set_head_detached(const annotated_commit_wrapper& commit);

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
