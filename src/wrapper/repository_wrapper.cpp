#include "../utils/git_exception.hpp"
#include "../wrapper/repository_wrapper.hpp"

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

reference_wrapper repository_wrapper::head() const
{
    git_reference* ref;
    throwIfError(git_repository_head(&ref, *this));
    return reference_wrapper(ref);
}

index_wrapper repository_wrapper::make_index()
{
    index_wrapper index = index_wrapper::init(*this);
    return index;
}

branch_wrapper repository_wrapper::create_branch(const std::string& name, bool force)
{
    return create_branch(name, commit_wrapper::from_reference_name(*this), force);
}

branch_wrapper repository_wrapper::create_branch(const std::string& name, const commit_wrapper& commit, bool force)
{
    git_reference* branch = nullptr;
    throwIfError(git_branch_create(&branch, *this, name.c_str(), commit, force));
    return branch_wrapper(branch);
}

branch_wrapper repository_wrapper::find_branch(const std::string& name)
{
    git_reference* branch = nullptr;
    throwIfError(git_branch_lookup(&branch, *this, name.c_str(), GIT_BRANCH_LOCAL));
    return branch_wrapper(branch);
}

branch_iterator repository_wrapper::iterate_branches(git_branch_t type) const
{
    git_branch_iterator* iter = nullptr;
    throwIfError(git_branch_iterator_new(&iter, *this, type));
    return branch_iterator(iter);
}
