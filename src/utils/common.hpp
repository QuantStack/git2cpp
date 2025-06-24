#pragma once

#include <string>
#include <utility>

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

template <class T>
class wrapper_base
{
public:
    using resource_type = T;

    wrapper_base(const wrapper_base&) = delete;
    wrapper_base& operator=(const wrapper_base&) = delete;

    wrapper_base(wrapper_base&& rhs)
        : p_ressource(rhs.p_ressource)
    {
        rhs.p_ressource = nullptr;
    }
    wrapper_base& operator=(wrapper_base&& rhs)
    {
        std::swap(p_ressource, rhs.p_ressource);
        return this;
    }

    operator resource_type*() const noexcept
    {
        return p_ressource;
    }

protected:
    // Allocation and deletion of p_ressource must be handled by inheriting class.
    wrapper_base() = default;
    ~wrapper_base() = default;
    resource_type* p_ressource = nullptr;
};

class libgit2_object : private noncopyable_nonmovable
{
public:

    libgit2_object();
    ~libgit2_object();
};

std::string get_current_git_path();
