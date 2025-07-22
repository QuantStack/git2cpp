#pragma once

#include <optional>
#include <string_view>

#include <git2.h>

#include "../wrapper/wrapper_base.hpp"

class commit_wrapper;

class branch_wrapper : public wrapper_base<git_reference>
{
public:

    using base_type = wrapper_base<git_reference>;

    ~branch_wrapper();

    branch_wrapper(branch_wrapper&&) = default;
    branch_wrapper& operator=(branch_wrapper&&) = default;

    std::string_view name() const;
    std::string_view reference_name() const;

private:
    
    explicit branch_wrapper(git_reference* ref);

    friend class repository_wrapper;
    friend class branch_iterator;
};

void delete_branch(branch_wrapper&& br);

// Rust / Python-like iterator instead of regular C++ iterator,
// because of the libgit2 API. Implementing the postfix increment
// operator of an input iterator would be overcomplicated.
class branch_iterator : public wrapper_base<git_branch_iterator>
{
public:

    using base_type = wrapper_base<git_branch_iterator>;

    ~branch_iterator();

    branch_iterator(branch_iterator&&) = default;
    branch_iterator& operator=(branch_iterator&&) = default;

    std::optional<branch_wrapper> next();

private:

    explicit branch_iterator(git_branch_iterator* iter);

    friend class repository_wrapper;
};
