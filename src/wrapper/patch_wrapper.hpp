#pragma once

#include <git2.h>
#include <string>

#include "../wrapper/wrapper_base.hpp"

class patch_wrapper : public wrapper_base<git_patch>
{
public:

    using base_type = wrapper_base<git_patch>;

    ~patch_wrapper();

    patch_wrapper(patch_wrapper&&) noexcept = default;
    patch_wrapper& operator=(patch_wrapper&&) noexcept = default;

    git_buf to_buf();
    static patch_wrapper patch_from_files(const std::string& path1, const std::string& file1_str, const std::string& path2, const std::string& file2_str, git_diff_options* diffopts);

private:

    patch_wrapper(git_patch* patch);
};
