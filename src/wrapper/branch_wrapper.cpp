#include "../utils/git_exception.hpp"
#include "../wrapper/branch_wrapper.hpp"
#include "../wrapper/commit_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"

#include <iostream>

branch_wrapper::branch_wrapper(git_reference* ref)
    : base_type(ref)
{    
}

branch_wrapper::~branch_wrapper()
{
    git_reference_free(p_resource);
    p_resource = nullptr;
}

std::string_view branch_wrapper::name() const
{
    const char* out = nullptr;
    throwIfError(git_branch_name(&out, *this));
    return std::string_view(out);
}

std::string_view branch_wrapper::reference_name() const
{
    const char* out = git_reference_name(*this);
    return out ? out : std::string_view();
}

void delete_branch(branch_wrapper&& branch)
{
    throwIfError(git_branch_delete(branch));
}

branch_iterator::branch_iterator(git_branch_iterator* iter)
    : base_type(iter)
{
}

branch_iterator::~branch_iterator()
{
    git_branch_iterator_free(p_resource);
    p_resource = nullptr;
}


std::optional<branch_wrapper> branch_iterator::next()
{
    git_reference* ref = nullptr;
    git_branch_t type;
    int res = git_branch_next(&ref, &type, p_resource);
    if (res == 0)
    {
        return branch_wrapper(ref);
    }
    else
    {
        return std::nullopt;
    }
}
