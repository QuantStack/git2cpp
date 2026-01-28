#pragma once

#include <git2.h>

#include <optional>

#include "../wrapper/annotated_commit_wrapper.hpp"
#include "../wrapper/signature_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"
#include "../wrapper/wrapper_base.hpp"

class rebase_wrapper : public wrapper_base<git_rebase>
{
public:

    using base_type = wrapper_base<git_rebase>;

    rebase_wrapper(rebase_wrapper&&) = default;
    rebase_wrapper& operator=(rebase_wrapper&&) = default;

    ~rebase_wrapper();

    bool next_operation();
    git_oid current_operation_id() const;
    std::size_t current_operation_index() const;
    std::size_t operation_entry_count() const;

    int commit(const signature_wrapper& author, const signature_wrapper& committer);
    void finish(const signature_wrapper& committer);
    void abort();

    static rebase_wrapper init
    (
        repository_wrapper& repo,
        const annotated_commit_wrapper& branch, 
        const annotated_commit_wrapper& upstream,
        const annotated_commit_wrapper* onto,
        const git_rebase_options& opts
    );
    static rebase_wrapper open(repository_wrapper& repo, const git_rebase_options& opts);

private:

    rebase_wrapper() = default;

    git_rebase_operation* p_operation = nullptr;
};

