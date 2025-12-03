#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"
#include "../wrapper/repository_wrapper.hpp"

class merge_subcommand
{
public:

    explicit merge_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:

    annotated_commit_list_wrapper resolve_heads(const repository_wrapper& repo);
    void create_merge_commit(
        repository_wrapper& repo,
        const index_wrapper& index,
        const std::vector<std::string>& branches_to_merge,
        const annotated_commit_list_wrapper& commits_to_merge,
        size_t num_commits_to_merge);

    std::vector<std::string> m_branches_to_merge;
    // bool m_no_ff = false;
    // bool m_commit = false;
    bool m_no_commit = false;
    bool m_abort = false;
    bool m_quit = false;
    bool m_continue = false;
};
