#pragma once

#include <string>
#include <utility>

class noncopiable_nonmovable
{
public:
    noncopiable_nonmovable(const noncopiable_nonmovable&) = delete;
    noncopiable_nonmovable& operator=(const noncopiable_nonmovable&) = delete;
    noncopiable_nonmovable(noncopiable_nonmovable&&) = delete;
    noncopiable_nonmovable& operator=(noncopiable_nonmovable&&) = delete;

protected:
    noncopiable_nonmovable() = default;
    ~noncopiable_nonmovable() = default;
};

template <class T>
class wrapper_base
{
public:
    using ressource_type = T;

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

    operator ressource_type*() const noexcept
    {
        return p_ressource;
    }

protected:
    // Allocation and deletion of p_ressource must be handled by inheriting class.
    wrapper_base() = default;
    ~wrapper_base() = default;
    ressource_type* p_ressource = nullptr;
};

class libgit2_object : private noncopiable_nonmovable
{
public:

    libgit2_object();
    ~libgit2_object();
};

std::string get_current_git_path();
