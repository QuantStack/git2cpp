#include "../wrapper/annotated_commit_wrapper.hpp"

annotated_commit_wrapper::annotated_commit_wrapper(git_annotated_commit* commit)
    : base_type(commit)
{
}

annotated_commit_wrapper::~annotated_commit_wrapper()
{
    git_annotated_commit_free(p_resource);
    p_resource = nullptr;
}

const git_oid& annotated_commit_wrapper::oid() const
{
    return *git_annotated_commit_id(p_resource);
}

std::string annotated_commit_wrapper::commit_oid_tostr() const
{
    char buf[GIT_OID_SHA1_HEXSIZE + 1];
    return git_oid_tostr(buf, sizeof(buf), &this->oid());
}

std::string_view annotated_commit_wrapper::reference_name() const
{
    const char* res = git_annotated_commit_ref(*this);
    return res ? res : std::string_view{};
}
