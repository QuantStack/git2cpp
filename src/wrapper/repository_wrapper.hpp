#pragma once

#include <git2.h>
#include <string>

#include "../utils/common.hpp"

class RepositoryWrapper : private noncopiable_nonmovable
{
public:

    ~RepositoryWrapper();

    // static RepositoryWrapper open(const std::string path);
    static void init(const std::string& directory, bool bare);

private:

    RepositoryWrapper();
    git_repository* p_repo;
};
