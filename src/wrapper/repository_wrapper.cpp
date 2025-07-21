#include "../utils/git_exception.hpp"
#include "../wrapper/repository_wrapper.hpp"

repository_wrapper::~repository_wrapper()
{
    git_repository_free(p_resource);
    p_resource=nullptr;
}

repository_wrapper repository_wrapper::open(std::string_view directory)
{
    repository_wrapper rw;
    throwIfError(git_repository_open(&(rw.p_resource), directory.data()));
    return rw;
}

repository_wrapper repository_wrapper::init(std::string_view directory, bool bare)
{
    repository_wrapper rw;
    throwIfError(git_repository_init(&(rw.p_resource), directory.data(), bare));
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

branch_wrapper repository_wrapper::create_branch(std::string_view name, bool force)
{
    return create_branch(name, find_commit(), force);
}

branch_wrapper repository_wrapper::create_branch(std::string_view name, const commit_wrapper& commit, bool force)
{
    git_reference* branch = nullptr;
    throwIfError(git_branch_create(&branch, *this, name.data(), commit, force));
    return branch_wrapper(branch);
}

branch_wrapper repository_wrapper::find_branch(std::string_view name)
{
    git_reference* branch = nullptr;
    throwIfError(git_branch_lookup(&branch, *this, name.data(), GIT_BRANCH_LOCAL));
    return branch_wrapper(branch);
}

branch_iterator repository_wrapper::iterate_branches(git_branch_t type) const
{
    git_branch_iterator* iter = nullptr;
    throwIfError(git_branch_iterator_new(&iter, *this, type));
    return branch_iterator(iter);
}


commit_wrapper repository_wrapper::find_commit(std::string_view ref_name) const
{
    git_oid oid_parent_commit;
    throwIfError(git_reference_name_to_id(&oid_parent_commit, *this, ref_name.data()));
    return find_commit(oid_parent_commit);
}

commit_wrapper repository_wrapper::find_commit(const git_oid& id) const
{
    git_commit* commit;
    throwIfError(git_commit_lookup(&commit, *this, &id));
    return commit_wrapper(commit);
}
