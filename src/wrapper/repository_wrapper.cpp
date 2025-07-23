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
    throw_if_error(git_repository_open(&(rw.p_resource), directory.data()));
    return rw;
}

repository_wrapper repository_wrapper::init(std::string_view directory, bool bare)
{
    repository_wrapper rw;
    throw_if_error(git_repository_init(&(rw.p_resource), directory.data(), bare));
    return rw;
}

repository_wrapper repository_wrapper::clone(std::string_view url, std::string_view path, const git_clone_options& opts)
{
    repository_wrapper rw;
    throw_if_error(git_clone(&(rw.p_resource), url.data(), path.data(), &opts));
    return rw;
}

git_repository_state_t repository_wrapper::state() const
{
    return git_repository_state_t(git_repository_state(*this));
}

reference_wrapper repository_wrapper::head() const
{
    git_reference* ref;
    throw_if_error(git_repository_head(&ref, *this));
    return reference_wrapper(ref);
}

reference_wrapper repository_wrapper::find_reference(std::string_view ref_name) const
{
    git_reference* ref;
    throw_if_error(git_reference_lookup(&ref, *this, ref_name.data()));
    return reference_wrapper(ref);
}

std::optional<reference_wrapper> repository_wrapper::find_reference_dwim(std::string_view ref_name) const
{
    git_reference* ref;
    int rc = git_reference_dwim(&ref, *this, ref_name.data());
    return rc == 0 ? std::make_optional(reference_wrapper(ref)) : std::nullopt;
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
    throw_if_error(git_branch_create(&branch, *this, name.data(), commit, force));
    return branch_wrapper(branch);
}

branch_wrapper repository_wrapper::create_branch(std::string_view name, const annotated_commit_wrapper& commit, bool force)
{
    git_reference* branch = nullptr;
    throw_if_error(git_branch_create_from_annotated(&branch, *this, name.data(), commit, force));
    return branch_wrapper(branch);
}

branch_wrapper repository_wrapper::find_branch(std::string_view name) const
{
    git_reference* branch = nullptr;
    throw_if_error(git_branch_lookup(&branch, *this, name.data(), GIT_BRANCH_LOCAL));
    return branch_wrapper(branch);
}

branch_iterator repository_wrapper::iterate_branches(git_branch_t type) const
{
    git_branch_iterator* iter = nullptr;
    throw_if_error(git_branch_iterator_new(&iter, *this, type));
    return branch_iterator(iter);
}

commit_wrapper repository_wrapper::find_commit(std::string_view ref_name) const
{
    git_oid oid_parent_commit;
    throw_if_error(git_reference_name_to_id(&oid_parent_commit, *this, ref_name.data()));
    return find_commit(oid_parent_commit);
}

commit_wrapper repository_wrapper::find_commit(const git_oid& id) const
{
    git_commit* commit;
    throw_if_error(git_commit_lookup(&commit, *this, &id));
    return commit_wrapper(commit);
}

annotated_commit_wrapper repository_wrapper::find_annotated_commit(const git_oid& id) const
{
    git_annotated_commit* commit;
    throw_if_error(git_annotated_commit_lookup(&commit, *this, &id));
    return annotated_commit_wrapper(commit);
}

std::optional<object_wrapper> repository_wrapper::revparse_single(std::string_view spec) const
{
    git_object* obj;
    int rc = git_revparse_single(&obj, *this, spec.data());
    return rc == 0 ? std::make_optional(object_wrapper(obj)) : std::nullopt;
}

void repository_wrapper::set_head(std::string_view ref_name)
{
    throw_if_error(git_repository_set_head(*this, ref_name.data()));
}

void repository_wrapper::set_head_detached(const annotated_commit_wrapper& commit)
{
    throw_if_error(git_repository_set_head_detached_from_annotated(*this, commit));
}
