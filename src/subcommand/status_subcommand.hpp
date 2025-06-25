#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class status_subcommand
{
public:

    explicit status_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:
    bool branch_flag = false;
    bool long_flag = false;
    bool short_flag = false;
};
