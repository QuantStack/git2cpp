#pragma once

#include <limits>

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"
#include "../wrapper/commit_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"

class log_subcommand
{
public:

    explicit log_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:

    void print_commit(repository_wrapper& repo, const commit_wrapper& commit);

    std::string m_format_flag;
    int m_max_count_flag = std::numeric_limits<int>::max();
    size_t m_abbrev = 7;
    bool m_abbrev_commit_flag = false;
    bool m_no_abbrev_commit_flag = false;
    bool m_oneline_flag = false;
};
