#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class add_subcommand
{
public:

    explicit add_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:
    bool m_all_flag = false;
    std::vector<std::string> m_add_files;
};
