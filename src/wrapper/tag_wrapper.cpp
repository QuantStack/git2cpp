#include "../wrapper/tag_wrapper.hpp"
#include <git2/tag.h>

tag_wrapper::tag_wrapper(git_tag* tag)
    : base_type(tag)
{
}

tag_wrapper::~tag_wrapper()
{
    git_tag_free(p_resource);
    p_resource = nullptr;
}

std::string tag_wrapper::name()
{
    return git_tag_name(*this);
}

std::string tag_wrapper::message()
{
    return git_tag_message(*this);
}
