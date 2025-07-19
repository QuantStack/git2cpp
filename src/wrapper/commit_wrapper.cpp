#include "../utils/git_exception.hpp"
#include "../wrapper/commit_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"

commit_wrapper::~commit_wrapper()
{
    git_commit_free(p_resource);
    p_resource = nullptr;
}


commit_wrapper commit_wrapper::from_reference_name(const repository_wrapper& repo, const std::string& ref_name)
{
    git_oid oid_parent_commit;
    throwIfError(git_reference_name_to_id(&oid_parent_commit, repo, ref_name.c_str()));

    commit_wrapper cw;
    throwIfError(git_commit_lookup(&(cw.p_resource), repo, &oid_parent_commit));
    return cw;
}
