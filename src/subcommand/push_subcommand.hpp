#pragma once

#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class push_subcommand
{
public:

    explicit push_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:

    std::string m_remote_name;
    std::vector<std::string> m_refspecs;
};
