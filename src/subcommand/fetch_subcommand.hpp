#pragma once

#include <string>

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class fetch_subcommand
{
public:

    explicit fetch_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:

    std::string m_remote_name;
};
