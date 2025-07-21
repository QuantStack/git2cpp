#pragma once

#include <string_view>

#include <git2.h>

#include "../wrapper/annotated_commit_wrapper.hpp"
#include "../wrapper/branch_wrapper.hpp"
#include "../wrapper/commit_wrapper.hpp"
#include "../wrapper/index_wrapper.hpp"
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

    // References
    reference_wrapper head() const;
    reference_wrapper find_reference(std::string_view ref_name) const;

    // Index
    index_wrapper make_index();

    // Branches
    branch_wrapper create_branch(std::string_view name, bool force);
    branch_wrapper create_branch(std::string_view name, const commit_wrapper& commit, bool force);
    branch_wrapper find_branch(std::string_view name) const;
    branch_iterator iterate_branches(git_branch_t type) const;

    // Commits
    commit_wrapper find_commit(std::string_view ref_name = "HEAD") const;
    commit_wrapper find_commit(const git_oid& id) const;

    // Annotated commits
    annotated_commit_wrapper find_annotated_commit(const git_oid& id) const;

private:

    repository_wrapper() = default;
};
