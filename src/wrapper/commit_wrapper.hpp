#pragma once

#include <string>

#include <git2.h>

#include "../wrapper/wrapper_base.hpp"

class repository_wrapper;

class commit_wrapper : public wrapper_base<git_commit>
{
public:

    ~commit_wrapper();

    commit_wrapper(commit_wrapper&&) noexcept = default;
    commit_wrapper& operator=(commit_wrapper&&) noexcept = default;

    static commit_wrapper
    from_reference_name(const repository_wrapper& repo, const std::string& ref_name = "HEAD");

private:

    commit_wrapper() = default;
};
