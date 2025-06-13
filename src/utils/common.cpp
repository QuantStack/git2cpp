#include <git2.h>

#include "common.hpp"

libgit2_object::libgit2_object()
{
    git_libgit2_init();
}

libgit2_object::~libgit2_object()
{
    git_libgit2_shutdown();
}
