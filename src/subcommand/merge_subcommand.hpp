#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class merge_subcommand
{
public:

    explicit merge_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:
    std::vector<std::string> m_branches_to_merge;
};
