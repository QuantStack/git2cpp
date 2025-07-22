#pragma once

#include <string>

#include <git2.h>

#include "../wrapper/wrapper_base.hpp"

class reference_wrapper : public wrapper_base<git_reference>
{
public:

    using base_type = wrapper_base<git_reference>;

    ~reference_wrapper();

    reference_wrapper(reference_wrapper&&) noexcept = default;
    reference_wrapper& operator=(reference_wrapper&&) noexcept = default;

    std::string short_name() const;
    bool is_remote() const;

private:

    reference_wrapper(git_reference* ref);

    friend class repository_wrapper;
};
