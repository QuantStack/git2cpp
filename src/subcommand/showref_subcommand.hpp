#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class showref_subcommand
{
public:

    explicit showref_subcommand(const libgit2_object&, CLI::App& app);
    void run();
};
