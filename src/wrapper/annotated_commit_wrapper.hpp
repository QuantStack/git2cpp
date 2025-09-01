#pragma once

#include <string_view>
#include <vector>

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

class annotated_commit_list_wrapper : public wrapper_base<git_annotated_commit*>
{
public:

    using base_type = wrapper_base<git_annotated_commit*>;

    explicit annotated_commit_list_wrapper(std::vector<annotated_commit_wrapper> annotated_commit_list);

    ~annotated_commit_list_wrapper();

    annotated_commit_list_wrapper(annotated_commit_list_wrapper&&) noexcept = default;
    annotated_commit_list_wrapper& operator=(annotated_commit_list_wrapper&&) noexcept = default;

    size_t size() const;
    annotated_commit_wrapper front();
    void print() const;

private:

    std::vector<annotated_commit_wrapper> m_annotated_commit_list;

};
