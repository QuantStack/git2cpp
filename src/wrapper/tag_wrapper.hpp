#pragma once

#include <git2.h>
#include <string>

#include "../wrapper/wrapper_base.hpp"

class tag_wrapper : public wrapper_base<git_tag>
{
public:

    using base_type = wrapper_base<git_tag>;

    ~tag_wrapper();

    tag_wrapper(tag_wrapper&&) noexcept = default;
    tag_wrapper& operator=(tag_wrapper&&) noexcept = default;

    std::string name();
    std::string message();

private:

     tag_wrapper(git_tag* tag);
};
