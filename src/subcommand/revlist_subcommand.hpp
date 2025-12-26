#pragma once

#include <CLI/CLI.hpp>
#include <string>

#include "../utils/common.hpp"

class revlist_subcommand
{
public:

    explicit revlist_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:

    std::string m_commit;
    int m_max_count_flag=std::numeric_limits<int>::max();

};
