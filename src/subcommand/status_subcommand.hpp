#pragma once

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"
#include "../wrapper/status_wrapper.hpp"

struct status_subcommand_options
{
    bool m_branch_flag = false;
    bool m_long_flag = false;
    bool m_short_flag = false;
};

class status_subcommand
{
public:

    explicit status_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:
    status_subcommand_options m_options;
};

void print_tobecommited(status_list_wrapper& sl, std::set<std::string> tracked_dir_set, bool is_long, bool is_coloured);
void print_notstagged(status_list_wrapper& sl, std::set<std::string> tracked_dir_set, bool is_long, bool is_coloured);
void print_tracking_info(repository_wrapper& repo, status_list_wrapper& sl, bool is_long);
void status_run(status_subcommand_options fl = {});
