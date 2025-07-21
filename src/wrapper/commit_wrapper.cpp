#include "../wrapper/commit_wrapper.hpp"

commit_wrapper::commit_wrapper(git_commit* commit)
    : base_type(commit)
{
}

commit_wrapper::~commit_wrapper()
{
    git_commit_free(p_resource);
    p_resource = nullptr;
}

const git_oid& commit_wrapper::oid() const
{
    return *git_commit_id(p_resource);
}

