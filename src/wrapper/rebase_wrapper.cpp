#include "rebase_wrapper.hpp"
#include "../utils/git_exception.hpp"

#include <iostream>

rebase_wrapper::~rebase_wrapper()
{
    git_rebase_free(p_resource);
    p_resource = nullptr;
}

bool rebase_wrapper::next_operation()
{
    int res = git_rebase_next(&p_operation, p_resource);
    return res == 0;
}

git_oid rebase_wrapper::current_operation_id() const
{
    return p_operation->id;
}
std::size_t rebase_wrapper::current_operation_index() const
{
    return git_rebase_operation_current(p_resource);
}

std::size_t rebase_wrapper::operation_entry_count() const
{
    return git_rebase_operation_entrycount(p_resource);
}

int rebase_wrapper::commit(const signature_wrapper& author, const signature_wrapper& committer)
{
    git_oid new_commit_oid;
    int res = git_rebase_commit(
        &new_commit_oid,
        p_resource,
        author,
        committer,
        nullptr,  // message encoding (NULL for default)
        nullptr); // message (NULL to use original)
    return res;
}

void rebase_wrapper::finish(const signature_wrapper& committer)
{
    throw_if_error(git_rebase_finish(p_resource, committer));
    p_resource = nullptr;
}

void rebase_wrapper::abort()
{
    throw_if_error(git_rebase_abort(p_resource));
}

rebase_wrapper rebase_wrapper::init
(
    repository_wrapper& repo,
    const annotated_commit_wrapper& branch, 
    const annotated_commit_wrapper& upstream,
    const annotated_commit_wrapper* onto,
    const git_rebase_options& opts
)
{
    rebase_wrapper wp;
    const git_annotated_commit* raw_onto = nullptr;
    if (onto)
    {
        raw_onto = *onto;
    }
    throw_if_error(git_rebase_init(&(wp.p_resource), repo, branch, upstream, raw_onto, &opts));
    return wp;
}

rebase_wrapper rebase_wrapper::open(repository_wrapper& repo, const git_rebase_options& opts)
{
    rebase_wrapper wp;
    throw_if_error(git_rebase_open(&(wp.p_resource), repo, &opts));
    return wp;
}
