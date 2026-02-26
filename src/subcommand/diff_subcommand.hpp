#pragma once

#include <CLI/CLI.hpp>
#include <string>

#include "../utils/common.hpp"
#include "../wrapper/diff_wrapper.hpp"

class diff_subcommand
{
public:

    explicit diff_subcommand(const libgit2_object&, CLI::App& app);
    void print_diff(diff_wrapper& diff, bool use_colour);
    void run();

private:

    std::vector<std::string> m_files;

    bool m_stat_flag = false;
    bool m_shortstat_flag = false;
    bool m_numstat_flag = false;
    bool m_summary_flag = false;
    bool m_name_only_flag = false;
    bool m_name_status_flag = false;
    bool m_raw_flag = false;

    bool m_cached_flag = false;
    bool m_no_index_flag = false;

    bool m_reverse_flag = false;
    bool m_text_flag = false;
    bool m_ignore_space_at_eol_flag = false;
    bool m_ignore_space_change_flag = false;
    bool m_ignore_all_space_flag = false;
    bool m_untracked_flag = false;
    bool m_patience_flag = false;
    bool m_minimal_flag = false;

    // int m_rename_threshold = 50;
    // bool m_find_renames_flag = false;
    // int m_copy_threshold = 50;
    // bool m_find_copies_flag = false;
    // bool m_find_copies_harder_flag = false;
    // bool m_break_rewrites_flag = false;

    int m_context_lines = 3;
    int m_interhunk_lines = 0;
    int m_abbrev = 7;

    bool m_colour_flag = true;
    bool m_no_colour_flag = false;
};

void print_stats(const diff_wrapper& diff, bool use_colour, bool stat_flag, bool shortstat_flag, bool numstat_flag, bool summary_flag);
