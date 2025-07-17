#pragma once

#include <string>

#include <git2.h>

#include "../wrapper/index_wrapper.hpp"
#include "../wrapper/wrapper_base.hpp"

class repository_wrapper : public wrapper_base<git_repository>
{
public:

    ~repository_wrapper();

    repository_wrapper(repository_wrapper&&) noexcept = default;
    repository_wrapper& operator=(repository_wrapper&&) noexcept = default;

    static repository_wrapper init(const std::string& directory, bool bare);
    static repository_wrapper open(const std::string& directory);
    index_wrapper make_index();

private:

    repository_wrapper() = default;
};
