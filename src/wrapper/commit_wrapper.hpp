#pragma once

#include <git2.h>
#include <vector>
#include <string>

#include "../wrapper/wrapper_base.hpp"

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

private:

    commit_wrapper(git_commit* commit);

    friend class repository_wrapper;
    friend class reference_wrapper;
};

class commit_list_wrapper : public wrapper_base<git_commit*>
{
public:

    using base_type = wrapper_base<git_commit*>;

    explicit commit_list_wrapper(std::vector<commit_wrapper> commit_list);

    ~commit_list_wrapper();

    commit_list_wrapper(commit_list_wrapper&&) noexcept = default;
    commit_list_wrapper& operator=(commit_list_wrapper&&) noexcept = default;

    size_t size() const;

private:

    std::vector<commit_wrapper> m_commit_list;

};
