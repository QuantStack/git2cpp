#include "../utils/git_exception.hpp"
#include "object_wrapper.hpp"
#include <git2/refs.h>
#include <git2/types.h>
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

const git_oid* reference_wrapper::target() const
{
    return git_reference_target(p_resource);
}

reference_wrapper reference_wrapper::write_new_ref(const git_oid target_oid)
{
    git_reference* new_ref;
    throw_if_error(git_reference_set_target(&new_ref, p_resource, &target_oid, NULL));
    return reference_wrapper(new_ref);
}
