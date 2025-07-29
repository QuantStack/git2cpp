#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class commit_subcommand
{
public:

    explicit commit_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:
    bool m_message_flag = true;   // TODO: change to false when a message can be provided if the "-m" flag is not provided
    std::string m_message;
};
