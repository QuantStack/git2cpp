#pragma once

#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class revparse_subcommand
{
public:

    explicit revparse_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:

    std::vector<std::string> m_revisions;
    std::vector<std::string> m_queries_in_order;
    bool m_is_bare_repository_flag = false;
    bool m_is_shallow_repository_flag = false;
};
