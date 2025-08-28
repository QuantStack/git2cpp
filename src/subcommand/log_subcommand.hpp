#pragma once

#include <CLI/CLI.hpp>
#include <limits>

#include "../utils/common.hpp"


class log_subcommand
{
public:

    explicit log_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:
    std::string m_format_flag;
    int m_max_count_flag=std::numeric_limits<int>::max();
    // bool m_oneline_flag = false;
};
