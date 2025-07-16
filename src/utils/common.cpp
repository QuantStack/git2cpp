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

git_strarray_wrapper::git_strarray_wrapper(std::vector<std::string> m_patterns)
    : m_patterns(std::move(m_patterns))
{
    init_str_array();
}

git_strarray_wrapper::git_strarray_wrapper(git_strarray_wrapper&& rhs)
    : m_patterns(std::move(rhs.m_patterns))
{
    init_str_array();
}

git_strarray_wrapper::~git_strarray_wrapper()
{
    delete[] m_array.strings;
}

git_strarray_wrapper::operator git_strarray*()
{
    return &m_array;
}

void git_strarray_wrapper::init_str_array()
{
    git_strarray_wrapper aw;
    git_strarray array{new char*[aw.m_patterns.size()], aw.m_patterns.size()};
    for (size_t i=0; i<aw.m_patterns.size(); ++i)
    {
        array.strings[i] = const_cast<char*>(aw.m_patterns[i].c_str());
    }
}
