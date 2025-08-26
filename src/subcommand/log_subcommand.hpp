#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"


class log_subcommand
{
public:

    explicit log_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:
    std::string m_format_flag;
    // int m_max_count_flag;
    // bool m_oneline_flag = false;
};
