#pragma once

#include <git2.h>
#include <string>

#include "../wrapper/wrapper_base.hpp"
#include "../wrapper/tree_wrapper.hpp"

class commit_wrapper;
using commit_list_wrapper = list_wrapper<commit_wrapper>;

class commit_wrapper : public wrapper_base<git_commit>
{
public:

    using base_type = wrapper_base<git_commit>;

    ~commit_wrapper();

    commit_wrapper(commit_wrapper&&) noexcept = default;
    commit_wrapper& operator=(commit_wrapper&&) noexcept = default;

    operator git_object*() const noexcept;

    const git_oid& oid() const;
    std::string commit_oid_tostr() const;

    std::string message() const;
    std::string summary() const;

    commit_wrapper get_parent(size_t i) const;
    commit_list_wrapper get_parents_list() const;

    tree_wrapper tree() const;

private:

    commit_wrapper(git_commit* commit);

    friend class repository_wrapper;
    friend class reference_wrapper;
};
