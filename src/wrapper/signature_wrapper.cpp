#include "../wrapper/repository_wrapper.hpp"
#include "../wrapper/signature_wrapper.hpp"
#include "../utils/git_exception.hpp"

signature_wrapper::~signature_wrapper()
{
    git_signature_free(p_resource);
    p_resource=nullptr;
}

signature_wrapper::author_committer_signatures signature_wrapper::get_default_signature_from_env(repository_wrapper& rw)
{
    signature_wrapper author;
    signature_wrapper committer;
    throw_if_error(git_signature_default_from_env(&(author.p_resource), &(committer.p_resource), rw));
    return {std::move(author), std::move(committer)};
}
