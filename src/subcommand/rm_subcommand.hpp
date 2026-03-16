#pragma once

#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class rm_subcommand
{
public:

    explicit rm_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:

    std::vector<std::string> m_pathspec;
    bool m_recursive = false;
};
