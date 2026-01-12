#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class config_subcommand
{
public:

    explicit config_subcommand(const libgit2_object&, CLI::App& app);
    void run_list();
    void run_set();
    void run_get();
    void run_unset();

    std::string m_name;
    std::string m_value;

    // TODO:
    // bool m_local_flag = false;
    // bool m_global_flag = false;
    // bool m_system_flag = false;
    // bool m_worktree_flag = false;
};
