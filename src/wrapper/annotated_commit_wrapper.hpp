#pragma once

#include <string_view>

#include <git2.h>

#include "../wrapper/wrapper_base.hpp"

class annotated_commit_wrapper : public wrapper_base<git_annotated_commit>
{
public:

    using base_type = wrapper_base<git_annotated_commit>;

    ~annotated_commit_wrapper();

    annotated_commit_wrapper(annotated_commit_wrapper&&) noexcept = default;
    annotated_commit_wrapper& operator=(annotated_commit_wrapper&&) noexcept = default;

    const git_oid& oid() const;
    std::string_view reference_name() const;

private:

    annotated_commit_wrapper(git_annotated_commit* commit);

    friend class repository_wrapper;
};

using annotated_commit_list_wrapper = list_wrapper<annotated_commit_wrapper>;
