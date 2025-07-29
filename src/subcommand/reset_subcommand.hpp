#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class reset_subcommand
{
public:

    explicit reset_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:
    std::string m_commit;
    bool m_soft_flag = false;
    bool m_mixed_flag = false;
    bool m_hard_flag = false;
};
