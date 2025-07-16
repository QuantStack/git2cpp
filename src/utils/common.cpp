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

git_strarray_wrapper::git_strarray_wrapper(std::vector<std::string> patterns)
    : m_patterns(std::move(patterns))
{
    init_str_array();
}

git_strarray_wrapper::git_strarray_wrapper(git_strarray_wrapper&& rhs)
    : m_patterns(std::move(rhs.m_patterns))
{
    init_str_array();
    rhs.reset_str_array();
}

git_strarray_wrapper& git_strarray_wrapper::operator=(git_strarray_wrapper&& rhs)
{
    using std::swap;
    swap(m_patterns, rhs.m_patterns);
    swap(m_array.strings, rhs.m_array.strings);
    swap(m_array.count, rhs.m_array.count);
    return *this;
}

git_strarray_wrapper::~git_strarray_wrapper()
{
    reset_str_array();
}

git_strarray_wrapper::operator git_strarray*()
{
    return &m_array;
}

void git_strarray_wrapper::reset_str_array()
{
    delete[] m_array.strings;
    m_array={nullptr, 0};
}

void git_strarray_wrapper::init_str_array()
{
    m_array.strings = new char*[m_patterns.size()];
    m_array.count = m_patterns.size();
    for (size_t i=0; i<m_patterns.size(); ++i)
    {
        m_array.strings[i] = const_cast<char*>(m_patterns[i].c_str());
    }
}
