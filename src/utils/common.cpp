#include <filesystem>

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

std::string get_current_git_path()
{
    return std::filesystem::current_path();  // TODO: make sure that it goes to the root
}

// // If directory not specified, uses cwd.
// sub->add_option("directory", directory, "info about directory arg")
//     ->check(CLI::ExistingDirectory | CLI::NonexistentPath)
//     ->default_val(std::filesystem::current_path());
