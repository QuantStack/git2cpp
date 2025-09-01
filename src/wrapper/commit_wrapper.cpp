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

std::string commit_wrapper::commit_oid_tostr() const
{
    char buf[GIT_OID_SHA1_HEXSIZE + 1];
    return git_oid_tostr(buf, sizeof(buf), &this->oid());
}

commit_list_wrapper::commit_list_wrapper(std::vector<commit_wrapper> commit_list)
{
    git_commit** p_resource = new git_commit*[m_commit_list.size()];
    for (size_t i=0; i<m_commit_list.size(); ++i)
    {
        p_resource[i] = m_commit_list[i];
    }
}

commit_list_wrapper::~commit_list_wrapper()
{
    delete[] p_resource;
    p_resource = nullptr;
}

size_t commit_list_wrapper::size() const
{
    return m_commit_list.size();
}
