#pragma once

#include <string>

#include <git2.h>

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

    static repository_wrapper init(const std::string& directory, bool bare);
    static repository_wrapper open(const std::string& directory);

    reference_wrapper head() const;

    index_wrapper make_index();

    branch_wrapper create_branch(const std::string& name, bool force);
    branch_wrapper create_branch(const std::string& name, const commit_wrapper& commit, bool force);

    branch_wrapper find_branch(const std::string& name);

    branch_iterator iterate_branches(git_branch_t type) const;

private:

    repository_wrapper() = default;
};
