#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

struct status_subcommand_options
{
    bool m_branch_flag = false;
    bool m_long_flag = false;
    bool m_short_flag = false;
};

class status_subcommand
{
public:

    explicit status_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:
    status_subcommand_options m_options;
};

void status_run(status_subcommand_options fl = {});
