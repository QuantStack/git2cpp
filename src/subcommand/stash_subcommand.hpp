#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class stash_subcommand
{
public:

    explicit stash_subcommand(const libgit2_object&, CLI::App& app);
    void run_push();
    void run_list();
    void run_pop();
    void run_apply();

    std::vector<std::string> m_options;
    std::string m_message = "";
    size_t m_index = 0;
};
