#include <filesystem>
#include <iostream>
#include <unistd.h>
#include <map>

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

const std::map<git_status_t, status_messages>& get_status_msg_map()
{
    static std::map<git_status_t, status_messages> status_msg_map =   //TODO : check spaces in short_mod
    {
        { GIT_STATUS_CURRENT, {"", ""} },
        { GIT_STATUS_INDEX_NEW, {"A  ", "\tnew file:   "} },
        { GIT_STATUS_INDEX_MODIFIED, {"M  ", "\tmodified:   "} },
        { GIT_STATUS_INDEX_DELETED, {"D  ", "\tdeleted:   "} },
        { GIT_STATUS_INDEX_RENAMED, {"R  ", "\trenamed:   "} },
        { GIT_STATUS_INDEX_TYPECHANGE, {"T  ", "\ttypechange:   "} },
        { GIT_STATUS_WT_NEW, {"?? ", "\t"} },
        { GIT_STATUS_WT_MODIFIED, {" M " , "\tmodified:   "} },
        { GIT_STATUS_WT_DELETED, {" D ", "\tdeleted:   "} },
        { GIT_STATUS_WT_TYPECHANGE, {" T ", "\ttypechange:   "} },
        { GIT_STATUS_WT_RENAMED, {" R ", "\trenamed:   "} },
        { GIT_STATUS_WT_UNREADABLE, {"", ""} },
        { GIT_STATUS_IGNORED, {"!! ", ""} },
        { GIT_STATUS_CONFLICTED, {"AA ", "\tboth added:   "} },
    };
    return status_msg_map;
}

status_messages get_status_msg(git_status_t st)
{
    return get_status_msg_map().find(st)->second;
}

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
