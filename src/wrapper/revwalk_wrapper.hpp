#pragma once

#include <git2.h>
#include <git2/types.h>

#include "../wrapper/wrapper_base.hpp"
#include "../wrapper/commit_wrapper.hpp"

class revwalk_wrapper : public wrapper_base<git_revwalk>
{
public:

    using base_type = wrapper_base<git_revwalk>;

    ~revwalk_wrapper();

    revwalk_wrapper(revwalk_wrapper&&) noexcept = default;
    revwalk_wrapper& operator=(revwalk_wrapper&&) noexcept = default;

    void push_head();
    void push(git_oid& commit_oid);
    int next(git_oid& commit_oid);

private:

    revwalk_wrapper(git_revwalk* walker);

    friend class repository_wrapper;
};
