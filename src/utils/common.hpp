#pragma once

#include <string>
#include <vector>

#include <git2.h>

class noncopyable_nonmovable
{
public:

    noncopyable_nonmovable(const noncopyable_nonmovable&) = delete;
    noncopyable_nonmovable& operator=(const noncopyable_nonmovable&) = delete;
    noncopyable_nonmovable(noncopyable_nonmovable&&) = delete;
    noncopyable_nonmovable& operator=(noncopyable_nonmovable&&) = delete;

protected:

    noncopyable_nonmovable() = default;
    ~noncopyable_nonmovable() = default;
};

class libgit2_object : private noncopyable_nonmovable
{
public:

    libgit2_object();
    ~libgit2_object();
};

std::string get_current_git_path();

struct status_messages
{
    std::string short_mod;
    std::string long_mod;
};

status_messages get_status_msg(git_status_t);

using stream_colour_fn = std::ostream& (*) (std::ostream&);

std::string read_file(const std::string& path);

std::vector<std::string> split_input_at_newlines(std::string_view str);

// Remove whitespace from start and end of a string.
std::string trim(const std::string& str);

std::string oid_to_hex(const git_oid& oid);
