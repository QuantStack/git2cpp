#pragma once

#include <string>

#include <git2.h>

#include "../wrapper/repository_wrapper.hpp"
#include "../wrapper/wrapper_base.hpp"

class commit_wrapper : public wrapper_base<git_commit>
{
public:

    ~commit_wrapper();

    commit_wrapper(commit_wrapper&&) noexcept = default;
    commit_wrapper& operator=(commit_wrapper&&) noexcept = default;

    static commit_wrapper
    last_commit(const repository_wrapper& repo, const std::string& ref_name = "HEAD");

private:

    commit_wrapper() = default;
};
