#include "src/utils/git_exception.hpp"
#include "repository_wrapper.hpp"


RepositoryWrapper::RepositoryWrapper()
        : p_repo(nullptr)
{}

RepositoryWrapper::~RepositoryWrapper()
{
    git_repository_free(p_repo);
    p_repo=nullptr;
}

// RepositoryWrapper::RepositoryWrapper open(const std::string path)
// {
//
// };

void RepositoryWrapper::init(const std::string& directory, bool bare)
{
    RepositoryWrapper rw;
    throwIfError(git_repository_init(&(rw.p_repo), directory.c_str(), bare));
}
