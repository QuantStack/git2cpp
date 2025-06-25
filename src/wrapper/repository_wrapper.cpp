#include "../utils/git_exception.hpp"
#include "repository_wrapper.hpp"


repository_wrapper::~repository_wrapper()
{
    git_repository_free(p_resource);
    p_resource=nullptr;
}

repository_wrapper repository_wrapper::open(const std::string& directory)
{
    repository_wrapper rw;
    throwIfError(git_repository_open(&(rw.p_resource), directory.c_str()));
    return rw;
}

repository_wrapper repository_wrapper::init(const std::string& directory, bool bare)
{
    repository_wrapper rw;
    throwIfError(git_repository_init(&(rw.p_resource), directory.c_str(), bare));
    return rw;
}
