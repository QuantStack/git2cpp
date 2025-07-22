#include "../utils/git_exception.hpp"
#include "../wrapper/refs_wrapper.hpp"

reference_wrapper::reference_wrapper(git_reference* ref)
    : base_type(ref)
{
}

reference_wrapper::~reference_wrapper()
{
    git_reference_free(p_resource);
    p_resource=nullptr;
}

std::string reference_wrapper::short_name() const
{
    return git_reference_shorthand(p_resource);
}

bool reference_wrapper::is_remote() const
{
    return git_reference_is_remote(*this);
}
