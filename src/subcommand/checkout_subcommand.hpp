#pragma once

#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"
#include "../wrapper/repository_wrapper.hpp"

class checkout_subcommand
{
public:

    explicit checkout_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:

    annotated_commit_wrapper
    create_local_branch(repository_wrapper& repo, const std::string_view target_name, bool force);

    void checkout_tree(
        const repository_wrapper& repo,
        const annotated_commit_wrapper& target_annotated_commit,
        const std::string_view target_name,
        const git_checkout_options& options
    );

    void update_head(
        repository_wrapper& repo,
        const annotated_commit_wrapper& target_annotated_commit,
        const std::string_view target_name
    );

    void checkout_head_files(
        const repository_wrapper& repo,
        const std::vector<std::string>& files,
        const git_checkout_options& options
    );

    void checkout_ref_files(
        const repository_wrapper& repo,
        const std::string_view tree_ish,
        const std::vector<std::string>& pathspecs,
        const git_checkout_options& options
    );

    std::vector<std::string> m_positional_args = {};
    bool m_create_flag = false;
    bool m_force_create_flag = false;
    bool m_force_checkout_flag = false;
};
