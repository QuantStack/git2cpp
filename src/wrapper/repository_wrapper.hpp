#pragma once

#include <string>

#include <git2.h>

#include "../utils/common.hpp"

class repository_wrapper : public wrapper_base<git_repository>
{
public:

    ~repository_wrapper();

    repository_wrapper(repository_wrapper&&) = default;
    repository_wrapper& operator=(repository_wrapper&&) = default;

    static repository_wrapper init(const std::string& directory, bool bare);
    static repository_wrapper open(const std::string& directory);

private:

    repository_wrapper() = default;
};
