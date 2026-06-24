#include "common.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <ranges>
#include <regex>
#include <sstream>

#include <git2.h>
#include <unistd.h>

#include "git_exception.hpp"

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
    static std::map<git_status_t, status_messages> status_msg_map =  // TODO : check spaces in short_mod
        {
            {GIT_STATUS_CURRENT, {"", ""}},
            {GIT_STATUS_INDEX_NEW, {"A  ", "\tnew file:   "}},
            {GIT_STATUS_INDEX_MODIFIED, {"M  ", "\tmodified:   "}},
            {GIT_STATUS_INDEX_DELETED, {"D  ", "\tdeleted:   "}},
            {GIT_STATUS_INDEX_RENAMED, {"R  ", "\trenamed:   "}},
            {GIT_STATUS_INDEX_TYPECHANGE, {"T  ", "\ttypechange:   "}},
            {GIT_STATUS_WT_NEW, {"?? ", "\t"}},
            {GIT_STATUS_WT_MODIFIED, {" M ", "\tmodified:   "}},
            {GIT_STATUS_WT_DELETED, {" D ", "\tdeleted:   "}},
            {GIT_STATUS_WT_TYPECHANGE, {" T ", "\ttypechange:   "}},
            {GIT_STATUS_WT_RENAMED, {" R ", "\trenamed:   "}},
            {GIT_STATUS_WT_UNREADABLE, {"", ""}},
            {GIT_STATUS_IGNORED, {"!! ", ""}},
            {GIT_STATUS_CONFLICTED, {"AA ", "\tboth added:   "}},
        };
    return status_msg_map;
}

status_messages get_status_msg(git_status_t st)
{
    return get_status_msg_map().find(st)->second;
}

std::string read_file(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        throw git_exception("error: Could not access " + path, git2cpp_error_code::GENERIC_ERROR);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::string> split_input_at_newlines(std::string_view str)
{
    auto split = str | std::ranges::views::split('\n')
                 | std::ranges::views::transform(
                     [](auto&& range)
                     {
                         return std::string(range.begin(), range.end());
                     }
                 );
    return std::vector<std::string>{split.begin(), split.end()};
}

std::string trim(const std::string& str)
{
    auto s = std::regex_replace(str, std::regex("^\\s+"), "");
    return std::regex_replace(s, std::regex("\\s+$"), "");
}

std::string oid_to_hex(const git_oid& oid)
{
    char oid_str[GIT_OID_SHA1_HEXSIZE + 1];
    git_oid_fmt(oid_str, &oid);
    oid_str[GIT_OID_SHA1_HEXSIZE] = '\0';
    return std::string(oid_str);
}
