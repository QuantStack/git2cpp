#include "../utils/git_exception.hpp"
#include "../wrapper/refs_wrapper.hpp"


reference_wrapper::~reference_wrapper()
{
    git_reference_free(p_resource);
    p_resource=nullptr;
}

std::string reference_wrapper::get_ref_name(const repository_wrapper& rw)
{
    reference_wrapper ref;
    throwIfError(git_repository_head(&(ref.p_resource), rw));
    return git_reference_shorthand(ref.p_resource);
}
