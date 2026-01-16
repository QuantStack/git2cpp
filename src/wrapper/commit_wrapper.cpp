#include "../wrapper/commit_wrapper.hpp"
#include <git2/commit.h>

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

std::string commit_wrapper::commit_oid_tostr() const
{
    char buf[GIT_OID_SHA1_HEXSIZE + 1];
    return git_oid_tostr(buf, sizeof(buf), &this->oid());
}

std::string commit_wrapper::summary() const
{
    return git_commit_summary(*this);
}

commit_list_wrapper commit_wrapper::get_parents_list() const
{
    size_t parent_count = git_commit_parentcount(*this);
    std::vector<commit_wrapper> parents_list;
    parents_list.reserve(parent_count);

    for (size_t i=0; i < parent_count; ++i)
    {
        git_commit* parent;
        git_commit_parent(&parent, *this, i);
        parents_list.push_back(commit_wrapper(parent));
    }
    return commit_list_wrapper(std::move(parents_list));
}
