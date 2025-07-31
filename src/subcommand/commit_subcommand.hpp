#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class commit_subcommand
{
public:

    explicit commit_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:
    std::string m_commit_message;
};
