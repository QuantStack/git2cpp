#include "../wrapper/annotated_commit_wrapper.hpp"
#include <iostream>

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

std::string_view annotated_commit_wrapper::reference_name() const
{
    const char* res = git_annotated_commit_ref(*this);
    return res ? res : std::string_view{};
}

annotated_commit_list_wrapper::annotated_commit_list_wrapper(std::vector<annotated_commit_wrapper> annotated_commit_list)
    : m_annotated_commit_list(std::move(annotated_commit_list))
{
    p_resource = new git_annotated_commit*[m_annotated_commit_list.size()];
    for (size_t i=0; i<m_annotated_commit_list.size(); ++i)
    {
        p_resource[i] = m_annotated_commit_list[i];
    }
}

annotated_commit_list_wrapper::~annotated_commit_list_wrapper()
{
    delete[] p_resource;
    p_resource = nullptr;
}

size_t annotated_commit_list_wrapper::size() const
{
    return m_annotated_commit_list.size();
}

annotated_commit_wrapper annotated_commit_list_wrapper::front()
{
    return annotated_commit_wrapper(std::move(m_annotated_commit_list.front()));
}

void annotated_commit_list_wrapper::print() const
{
    std::cout << "aclw - p_resource = " << p_resource <<std::endl;
    for (size_t i=0; i<m_annotated_commit_list.size(); ++i)
    {
        std::cout << "aclw - p_resource[i] = :" << p_resource[i] << std::endl;
    }
}
