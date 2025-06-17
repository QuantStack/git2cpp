#pragma once

#include <string>

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class init_subcommand
{
public:

    explicit init_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:
    bool bare;
    std::string directory;
};
