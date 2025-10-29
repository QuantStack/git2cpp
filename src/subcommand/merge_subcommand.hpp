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

    std::vector<std::string> m_branches_to_merge;
};
