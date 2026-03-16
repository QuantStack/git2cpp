#pragma once

#include <string>

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class mv_subcommand
{
public:

    explicit mv_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:

    std::string m_source_path;
    std::string m_destination_path;
    bool m_force = false;
};
