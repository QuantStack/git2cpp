#include "../wrapper/repository_wrapper.hpp"
#include "../wrapper/signature_wrapper.hpp"
#include "../utils/git_exception.hpp"
#include <git2/types.h>

signature_wrapper::~signature_wrapper()
{
    if (m_ownership)
    {
        git_signature_free(p_resource);
    }
    p_resource=nullptr;
}

std::string_view signature_wrapper::name() const
{
    return p_resource->name;
}

std::string_view signature_wrapper::email() const
{
    return p_resource->email;
}

git_time signature_wrapper::when() const
{
    return p_resource->when;
}

signature_wrapper::author_committer_signatures signature_wrapper::get_default_signature_from_env(repository_wrapper& rw)
{
    signature_wrapper author;
    signature_wrapper committer;
    throw_if_error(git_signature_default_from_env(&(author.p_resource), &(committer.p_resource), rw));
    return {std::move(author), std::move(committer)};
}

signature_wrapper signature_wrapper::get_commit_author(const commit_wrapper& cw)
{
    signature_wrapper author;
    author.p_resource = const_cast<git_signature*>(git_commit_author(cw));
    author.m_ownership = false;
    return author;
}

signature_wrapper signature_wrapper::get_commit_committer(const commit_wrapper& cw)
{
    signature_wrapper committer;
    committer.p_resource = const_cast<git_signature*>(git_commit_committer(cw));
    committer.m_ownership = false;
    return committer;
}

signature_wrapper signature_wrapper::signature_now(std::string name, std::string email)
{
    signature_wrapper sw;
    git_signature* signature;
    throw_if_error(git_signature_now(&signature, name.c_str(), email.c_str()));
    sw.p_resource = signature;
    sw.m_ownership = true;
    return sw;
}

signature_wrapper::author_committer_signatures signature_wrapper::signature_now(std::string author_name, std::string author_email, std::string committer_name, std::string committer_email)
{
    signature_wrapper author_sig = signature_now(author_name.c_str(), author_email.c_str());
    signature_wrapper cmt_sig = signature_now(committer_name.c_str(), committer_email.c_str());
    // Deep copy of "when", which contains only copiable values, not pointers
    cmt_sig.p_resource->when = author_sig.p_resource->when;
    return std::pair(std::move(author_sig), std::move(cmt_sig));
}
