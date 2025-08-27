#pragma once

#include <git2.h>

#include "../wrapper/repository_wrapper.hpp"
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
};
