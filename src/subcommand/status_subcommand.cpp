#include <iostream>
#include <ostream>
#include <set>
#include <string>

#include <git2.h>
#include <termcolor/termcolor.hpp>

#include "status_subcommand.hpp"
#include "../wrapper/status_wrapper.hpp"
#include "../wrapper/refs_wrapper.hpp"


status_subcommand::status_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("status", "Show modified files in working directory, staged for your next commit");

    sub->add_flag("-s,--short", m_short_flag, "Give the output in the short-format.");
    sub->add_flag("--long", m_long_flag, "Give the output in the long-format. This is the default.");
    // sub->add_flag("--porcelain[=<version>]", porcelain, "Give the output in an easy-to-parse format for scripts.
    //     This is similar to the short output, but will remain stable across Git versions and regardless of user configuration.
    //     See below for details. The version parameter is used to specify the format version. This is optional and defaults
    //     to the original version v1 format.");
    sub->add_flag("-b,--branch", m_branch_flag, "Show the branch and tracking info even in short-format.");

    sub->callback([this]() { this->run(); });
};

const std::string untracked_header = "Untracked files:\n  (use \"git add <file>...\" to include in what will be committed)\n";
const std::string tobecommited_header = "Changes to be committed:\n";
//   (use \"git restore --staged <file>...\" to unstage)\n
//   (use \"git reset HEAD <file>...\" to unstage)\n";
// const std::string ignored_header = "Ignored files:\n  (use \"git add -f <file>...\" to include in what will be committed)\n";
const std::string notstagged_header = "Changes not staged for commit:\n  (use \"git add <file>...\" to update what will be committed)\n";
//  (use \"git restore <file>...\" to discard changes in working directory)\n
//  (use \"git checkout -- <file>...\" to discard changes in working directory)\n"
const std::string nothingtocommit_msg = "No changes added to commit (use \"git add\" and/or \"git commit -a\")";
const std::string uptodate_msg = "Nothing to commit, working tree clean.";
const std::string nothingtocommit_untrackedfiles_msg = "Nothing added to commit but untracked files present (use \"git add\" to track)";
//   no changes added to commit (use "git add" and/or "git commit -a")

struct status_messages
{
    std::string short_mod;
    std::string long_mod;
};

const std::map<git_status_t, status_messages> status_msg_map =   //TODO : check spaces in short_mod
{
    { GIT_STATUS_CURRENT, {"", ""} },
    { GIT_STATUS_INDEX_NEW, {"A  ", "\tnew file:"} },
    { GIT_STATUS_INDEX_MODIFIED, {"M  ", "\tmodified:"} },
    { GIT_STATUS_INDEX_DELETED, {"D  ", "\tdeleted:"} },
    { GIT_STATUS_INDEX_RENAMED, {"R  ", "\trenamed:"} },
    { GIT_STATUS_INDEX_TYPECHANGE, {"T  ", "\ttypechange:"} },
    { GIT_STATUS_WT_NEW, {"?? ", "      "} },
    { GIT_STATUS_WT_MODIFIED, {" M " , "\tmodified:"} },
    { GIT_STATUS_WT_DELETED, {" D ", "\tdeleted:"} },
    { GIT_STATUS_WT_TYPECHANGE, {" T ", "\ttypechange:"} },
    { GIT_STATUS_WT_RENAMED, {" R ", "\trenamed:"} },
    { GIT_STATUS_WT_UNREADABLE, {"", ""} },
    { GIT_STATUS_IGNORED, {"!! ", ""} },
    { GIT_STATUS_CONFLICTED, {"", ""} },
};

enum class output_format
{
    DEFAULT = 0,
    LONG = 1,
    SHORT = 2
};

struct print_entry
{
    std::string status;
    std::string item;
};

std::string get_print_status(git_status_t status, output_format of)
{
    std::string entry_status;
    if ((of == output_format::DEFAULT) || (of == output_format::LONG))
    {
        if (status == GIT_STATUS_WT_NEW)
        {
            entry_status = status_msg_map.at(status).long_mod + "\t";
        }
        else
        {
            entry_status = status_msg_map.at(status).long_mod + "   ";
        }
    }
    else if (of == output_format::SHORT)
    {
        entry_status = status_msg_map.at(status).short_mod;
    }
    return entry_status;
}

void update_tracked_dir_set(const char* path, std::set<std::string>* tracked_dir_set = nullptr)
{
    if (tracked_dir_set)
    {
        const size_t first_slash_idx = std::string_view(path).find('/');
        if (std::string::npos != first_slash_idx)
        {
            auto directory = std::string_view(path).substr(0, first_slash_idx);
            tracked_dir_set->insert(std::string(directory));
        }
    }
}

std::string get_print_item(const char* old_path, const char* new_path)
{
    std::string entry_item;
    if (old_path && new_path && std::strcmp(old_path, new_path))
    {
        entry_item = std::string(old_path) + " -> " + std::string(new_path);
    }
    else
    {
        entry_item = old_path ? old_path : new_path;
    }
    return entry_item;
}

std::vector<print_entry> get_entries_to_print(git_status_t status, status_list_wrapper& sl,
    bool head_selector, output_format of, std::set<std::string>* tracked_dir_set = nullptr)
{
    std::vector<print_entry> entries_to_print{};
    const auto& entry_list = sl.get_entry_list(status);
    if (entry_list.empty())
    {
        return entries_to_print;
    }

    for (auto* entry : entry_list)
    {
        git_diff_delta* diff_delta = head_selector ? entry->head_to_index : entry->index_to_workdir;
        const char* old_path = diff_delta->old_file.path;
        const char* new_path = diff_delta->new_file.path;

        update_tracked_dir_set(old_path, tracked_dir_set);

        print_entry e = { get_print_status(status, of), get_print_item(old_path, new_path)};

        entries_to_print.push_back(std::move(e));
    }
    return entries_to_print;
}

void print_entries(std::vector<print_entry> entries_to_print, bool is_long, stream_colour_fn colour)
{
    for (auto e: entries_to_print)
    {
        if (is_long)
        {
            std::cout << colour << e.status << e.item << termcolor::reset << std::endl;
        }
        else
        {
            std::cout << colour << e.status << termcolor::reset << e.item << std::endl;
        }
    }
}

void print_not_tracked(const std::vector<print_entry>& entries_to_print, const std::set<std::string>& tracked_dir_set,
        std::set<std::string>& untracked_dir_set, bool is_long, stream_colour_fn colour)
{
    std::vector<print_entry> not_tracked_entries_to_print{};
    for (auto e: entries_to_print)
    {
        const size_t first_slash_idx = e.item.find('/');
        if (std::string::npos != first_slash_idx)
        {
            auto directory = e.item.substr(0, first_slash_idx);
            auto directory_print = e.item.substr(0, first_slash_idx) + "/";
            if (tracked_dir_set.contains(directory))
            {
                not_tracked_entries_to_print.push_back(e);
            }
            else
            {
                if (untracked_dir_set.contains(directory))
                {}
                else
                {
                    not_tracked_entries_to_print.push_back({e.status, directory_print});
                    untracked_dir_set.insert(std::string(directory));
                }
            }
        }
        else
        {
            not_tracked_entries_to_print.push_back(e);
        }
    }
    print_entries(not_tracked_entries_to_print, is_long, colour);
}

void status_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);
    auto sl = status_list_wrapper::status_list(repo);
    auto branch_name = repo.head().short_name();

    std::set<std::string> tracked_dir_set{};
    std::set<std::string> untracked_dir_set{};
    std::vector<std::string> untracked_to_print{};
    std::vector<std::string> ignored_to_print{};

    output_format of = output_format::DEFAULT;
    if (m_short_flag)
    {
        of = output_format::SHORT;
    }
    if (m_long_flag)
    {
        of = output_format::LONG;
    }
    // else if (porcelain_format)
    // {
    //     output_format = 3;
    // }

    bool is_long;
    is_long = ((of == output_format::DEFAULT) || (of == output_format::LONG));
    if (is_long)
    {
        std::cout  << "On branch " << branch_name << std::endl;
    }
    else
    {
        if (m_branch_flag)
        {
            std::cout  << "## " << branch_name << std::endl;
        }
    }

    if (sl.has_tobecommited_header())
    {
        stream_colour_fn colour = termcolor::green;
        if (is_long)
        {
            std::cout << tobecommited_header;
        }
        print_entries(get_entries_to_print(GIT_STATUS_INDEX_NEW, sl, true, of, &tracked_dir_set), is_long, colour);
        print_entries(get_entries_to_print(GIT_STATUS_INDEX_MODIFIED, sl, true, of, &tracked_dir_set), is_long, colour);
        print_entries(get_entries_to_print(GIT_STATUS_INDEX_DELETED, sl, true, of, &tracked_dir_set), is_long, colour);
        print_entries(get_entries_to_print(GIT_STATUS_INDEX_RENAMED, sl, true, of, &tracked_dir_set), is_long, colour);
        print_entries(get_entries_to_print(GIT_STATUS_INDEX_TYPECHANGE, sl, true, of, &tracked_dir_set), is_long, colour);
        if (is_long)
        {
            std::cout << std::endl;
        }
    }

    if (sl.has_notstagged_header())
    {
        stream_colour_fn colour = termcolor::red;
        if (is_long)
        {
            std::cout << notstagged_header;
        }
        print_entries(get_entries_to_print(GIT_STATUS_WT_MODIFIED, sl, false, of, &tracked_dir_set), is_long, colour);
        print_entries(get_entries_to_print(GIT_STATUS_WT_DELETED, sl, false, of, &tracked_dir_set), is_long, colour);
        print_entries(get_entries_to_print(GIT_STATUS_WT_TYPECHANGE, sl, false, of, &tracked_dir_set), is_long, colour);
        print_entries(get_entries_to_print(GIT_STATUS_WT_RENAMED, sl, false, of, &tracked_dir_set), is_long, colour);
        if (is_long)
        {
            std::cout << std::endl;
        }
    }

    if (sl.has_untracked_header())
    {
        stream_colour_fn colour = termcolor::red;
        if (is_long)
        {
            std::cout << untracked_header;
        }
        print_not_tracked(get_entries_to_print(GIT_STATUS_WT_NEW, sl, false, of), tracked_dir_set, untracked_dir_set, is_long, colour);
        if (is_long)
        {
            std::cout << std::endl;
        }
    }

    // if (sl.has_ignored_header())
    // {
    //     stream_colour_fn colour = termcolor::red;
    //     if (is_long)
    //     {
    //         std::cout << ignored_header;
    //     }
    //     print_not_tracked(get_entries_to_print(GIT_STATUS_IGNORED, sl, false, of), tracked_dir_set, untracked_dir_set, is_long, colour);
    //     if (is_long)
    //     {
    //         std::cout << std::endl;
    //     }
    // }

    if (!sl.has_tobecommited_header() && (sl.has_notstagged_header() || sl.has_untracked_header()))
    {
        if (sl.has_untracked_header())
        {
            std::cout << nothingtocommit_untrackedfiles_msg << std::endl;
        }
        else
        {
            std::cout << nothingtocommit_msg << std::endl;
        }
    }

    if (!sl.has_notstagged_header() && !sl.has_untracked_header())
    {
        std::cout << uptodate_msg << std::endl;
    }
}
