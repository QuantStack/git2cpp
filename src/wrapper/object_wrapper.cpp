#include "../wrapper/object_wrapper.hpp"

object_wrapper::object_wrapper(git_object* obj)
    : base_type(obj)
{
}

object_wrapper::~object_wrapper()
{
    git_object_free(p_resource);
    p_resource = nullptr;
}

const git_oid& object_wrapper::oid() const
{
    return *git_object_id(*this);
}
