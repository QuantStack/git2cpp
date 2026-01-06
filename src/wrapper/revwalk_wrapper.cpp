#include <git2/index.h>
#include <git2/types.h>

#include "revwalk_wrapper.hpp"
#include "../utils/git_exception.hpp"

revwalk_wrapper::revwalk_wrapper(git_revwalk* walker)
    : base_type(walker)
{
}

revwalk_wrapper::~revwalk_wrapper()
{
    git_revwalk_free(p_resource);
    p_resource=nullptr;
}

void revwalk_wrapper::push_head()
{
    throw_if_error(git_revwalk_push_head(*this));
}

void revwalk_wrapper::push(git_oid& commit_oid)
{
    throw_if_error(git_revwalk_push(*this, &commit_oid));
}

int revwalk_wrapper::next(git_oid& commit_oid)
{
    return git_revwalk_next(&commit_oid, *this);
}
