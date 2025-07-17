#pragma once

 #include <string>

#include <git2.h>

#include "../wrapper/repository_wrapper.hpp"
#include "../wrapper/wrapper_base.hpp"

class reference_wrapper : public wrapper_base<git_reference>
{
public:

    ~reference_wrapper();

    reference_wrapper(reference_wrapper&&) noexcept = default;
    reference_wrapper& operator=(reference_wrapper&&) noexcept = default;

    static std::string get_ref_name(const repository_wrapper& repo);

private:

    reference_wrapper() = default;
};
