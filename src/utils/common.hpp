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

using stream_colour_fn = std::ostream& (*)(std::ostream&);

class git_strarray_wrapper
{
public:
    git_strarray_wrapper()
        : m_patterns{}
        , m_array{nullptr, 0}
    {}
    git_strarray_wrapper(std::vector<std::string> patterns);

    git_strarray_wrapper(const git_strarray_wrapper&) = delete;
    git_strarray_wrapper& operator=(const git_strarray_wrapper&) = delete;

    git_strarray_wrapper(git_strarray_wrapper&& rhs);
    git_strarray_wrapper& operator=(git_strarray_wrapper&&);

    ~git_strarray_wrapper();

    operator git_strarray*();

private:
    std::vector<std::string> m_patterns;
    git_strarray m_array;

    void reset_str_array();
    void init_str_array();
};

std::string read_file(const std::string& path);
