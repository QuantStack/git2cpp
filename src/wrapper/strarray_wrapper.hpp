#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <git2.h>

// Wrapper of git_strarray that frees the contained
// strings (i.e. calls git_strarray_dispose) upon destruction.
class strarray_owned_wrapper
{
public:

    strarray_owned_wrapper();
    explicit strarray_owned_wrapper(git_strarray&& arr);

    strarray_owned_wrapper(const strarray_owned_wrapper&) = delete;
    strarray_owned_wrapper operator=(const strarray_owned_wrapper&) = delete;

    strarray_owned_wrapper(strarray_owned_wrapper&& rhs);
    strarray_owned_wrapper& operator=(strarray_owned_wrapper&& rhs);

    ~strarray_owned_wrapper();

    operator git_strarray*();

    size_t size() const;

    std::string_view operator[](size_t i) const;

private:

    git_strarray m_array;
};

// Wrapper of git_strarray containing pointers to strings
// stored in a stnadard container. Does not free them upon
// destruction.
class  strarray_view_wrapper
{
public:

    strarray_view_wrapper()
        : m_patterns{}
        , m_array{nullptr, 0}
    {
    }

    strarray_view_wrapper(std::vector<std::string> patterns);

    strarray_view_wrapper(const strarray_view_wrapper&) = delete;
    strarray_view_wrapper& operator=(const strarray_view_wrapper&) = delete;

    strarray_view_wrapper(strarray_view_wrapper&& rhs);
    strarray_view_wrapper& operator=(strarray_view_wrapper&& rhs);

    ~strarray_view_wrapper();

    operator git_strarray*();

    size_t size() const;

private:

    std::vector<std::string> m_patterns;
    git_strarray m_array;

    void reset_str_array();
    void init_str_array();
};
