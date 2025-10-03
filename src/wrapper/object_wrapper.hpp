#pragma once

#include <git2.h>

#include "../wrapper/wrapper_base.hpp"

class object_wrapper : public wrapper_base<git_object>
{
public:

    using base_type = wrapper_base<git_object>;

    ~object_wrapper();

    object_wrapper(object_wrapper&&) noexcept = default;
    object_wrapper& operator=(object_wrapper&&) noexcept = default;

    const git_oid& oid() const;

    operator git_commit*() const noexcept;

private:

    object_wrapper(git_object* obj);

    friend class repository_wrapper;
    friend class reference_wrapper;
};
