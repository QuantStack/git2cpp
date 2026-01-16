#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

struct status_subcommand_flags
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
    status_subcommand_flags m_fl;
};

void status_run(status_subcommand_flags fl = {});
