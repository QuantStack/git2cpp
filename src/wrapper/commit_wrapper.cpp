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

commit_wrapper::operator git_object*() const noexcept
{
    return reinterpret_cast<git_object*>(p_resource);
}

const git_oid& commit_wrapper::oid() const
{
    return *git_commit_id(p_resource);
}

