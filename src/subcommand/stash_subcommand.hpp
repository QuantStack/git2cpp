#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"
#include "../wrapper/repository_wrapper.hpp"

class stash_subcommand
{
public:

    explicit stash_subcommand(const libgit2_object&, CLI::App& app);
    git_oid resolve_stash_commit(repository_wrapper& repo);
    void run_push();
    void run_list();
    void run_pop();
    void run_apply();
    void run_show();

private:

    std::vector<std::string> m_options;
    std::string m_message = "";
    size_t m_index = 0;

    bool m_stat_flag = false;
    bool m_shortstat_flag = false;
    bool m_numstat_flag = false;
    bool m_summary_flag = false;
};
