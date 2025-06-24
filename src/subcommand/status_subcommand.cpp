#include <iostream>
#include <ostream>
#include <string>

#include <git2.h>

#include "status_subcommand.hpp"
#include "../wrapper/status_wrapper.hpp"

status_subcommand::status_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("status", "Show modified files in working directory, staged for your next commit");
    // Displays paths that have differences between the index file and the current HEAD commit,
    // paths that have differences between the working tree and the index file, and paths in the
    // working tree that are not tracked by Git (and are not ignored by gitignore[5]).
    // The first are what you would commit by running git commit;
    // the second and third are what you could commit by running git add before running git commit.

    sub->add_flag("-s,--short", short_flag, "Give the output in the short-format.");
    sub->add_flag("--long", long_flag, "Give the output in the long-format. This is the default.");
    // sub->add_flag("--porcelain[=<version>]", porcelain, "Give the output in an easy-to-parse format for scripts.
    //     This is similar to the short output, but will remain stable across Git versions and regardless of user configuration.
    //     See below for details. The version parameter is used to specify the format version. This is optional and defaults
    //     to the original version v1 format.");

    sub->callback([this]() { this->run(); });
};

const std::string untracked_header = "Untracked files:\n";
// "Untracked files:\n  (use \"git add <file>...\" to include in what will be committed)";
const std::string tobecommited_header = "Changes to be committed:\n";
// "Changes to be committed:\n  (use \"git reset HEAD <file>...\" to unstage)";
const std::string ignored_header = "Ignored files:\n";
// "Ignored files:\n  (use \"git add -f <file>...\" to include in what will be committed)"
const std::string notstagged_header = "Changes not staged for commit:\n";
// "Changes not staged for commit:\n  (use \"git add%s <file>...\" to update what will be committed)\n  (use \"git checkout -- <file>...\" to discard changes in working directory)"
const std::string nothingtocommit_message = "No changes added to commit";
// "No changes added to commit (use \"git add\" and/or \"git commit -a\")"

struct status_messages
{
    std::string short_mod;
    std::string long_mod;
};

const std::map<git_status_t, status_messages> status_msg_map =   //TODO : check spaces in short_mod
{
    { GIT_STATUS_CURRENT, {"", ""} },
    { GIT_STATUS_INDEX_NEW, {"A  ", "\t new file:"} },
    { GIT_STATUS_INDEX_MODIFIED, {"M  ", "\t modified:"} },
    { GIT_STATUS_INDEX_DELETED, {"D  ", "\t deleted:"} },
    { GIT_STATUS_INDEX_RENAMED, {"R  ", "\t renamed:"} },
    { GIT_STATUS_INDEX_TYPECHANGE, {"T  ", "\t typechange:"} },
    { GIT_STATUS_WT_NEW, {"?? ", ""} },
    { GIT_STATUS_WT_MODIFIED, {" M " , "\t modified:"} },
    { GIT_STATUS_WT_DELETED, {" D ", "\t deleted:"} },
    { GIT_STATUS_WT_TYPECHANGE, {" T ", "\t typechange:"} },
    { GIT_STATUS_WT_RENAMED, {" R ", "\t renamed:"} },
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

void print_entries(git_status_t status, status_list_wrapper& sl, bool head_selector, output_format of)   // TODO: add different mods
{
    const auto& entry_list = sl.get_entry_list(status);
    if (!entry_list.empty())
    {
        for (auto* entry : entry_list)
        {
            if ((of == output_format::DEFAULT) || (of == output_format::LONG))
            {
                std::cout << status_msg_map.at(status).long_mod << "\t";
            }
            else if (of == output_format::SHORT)
            {
                std::cout << status_msg_map.at(status).short_mod;
            }

            git_diff_delta* diff_delta;
            if (head_selector)
            {
                diff_delta = entry->head_to_index;
            }
            else
            {
                diff_delta = entry->index_to_workdir;
            }
            const char* old_path = diff_delta->old_file.path;
            const char* new_path = diff_delta->new_file.path;
            if (old_path && new_path && std::strcmp(old_path, new_path))
            {
                std::cout << old_path << " -> " << new_path << std::endl;
            }
            else
            {
                if (old_path)
                {
                    std::cout << old_path  << std::endl;
                }
                else
                {
                    std::cout << new_path  << std::endl;
                }
            }
        }
    }
    else
    {}
}

void status_subcommand::run()
{
    auto directory = get_current_git_path();
    auto bare = false;
    auto repo = repository_wrapper::init(directory, bare);
    auto sl = status_list_wrapper::status_list(repo);

    // TODO: add branch info

    output_format of = output_format::DEFAULT;
    if (short_flag)
    {
        of = output_format::SHORT;
    }
    if (long_flag)
    {
        of = output_format::LONG;
    }
    // else if (porcelain_format)
    // {
    //     output_format = 3;
    // }

    bool is_long;
    is_long = ((of == output_format::DEFAULT) || (of == output_format::LONG));
    if (sl.has_tobecommited_header())
    {
        if (is_long)
        {
            std::cout << tobecommited_header << std::endl;
        }
        print_entries(GIT_STATUS_INDEX_NEW, sl, true, of);
        print_entries(GIT_STATUS_INDEX_MODIFIED, sl, true, of);
        print_entries(GIT_STATUS_INDEX_DELETED, sl, true, of);
        print_entries(GIT_STATUS_INDEX_RENAMED, sl, true, of);
        print_entries(GIT_STATUS_INDEX_TYPECHANGE, sl, true, of);
        if (is_long)
        {
            std::cout << std::endl;
        }
    }

    if (sl.has_notstagged_header())
    {
        if (is_long)
        {
            std::cout << notstagged_header << std::endl;
        }
        print_entries(GIT_STATUS_WT_MODIFIED, sl, false, of);
        print_entries(GIT_STATUS_WT_DELETED, sl, false, of);
        print_entries(GIT_STATUS_WT_TYPECHANGE, sl, false, of);
        print_entries(GIT_STATUS_WT_RENAMED, sl, false, of);
        if (is_long)
        {
            std::cout << std::endl;
        }
    }

    if (sl.has_untracked_header())
    {
        if (is_long)
        {
            std::cout << untracked_header << std::endl;
        }
        print_entries(GIT_STATUS_WT_NEW, sl, false, of);
        if (is_long)
        {
            std::cout << std::endl;
        }
    }

    if (sl.has_ignored_header())
    {
        if (is_long)
        {
            std::cout << ignored_header << std::endl;
        }
        print_entries(GIT_STATUS_IGNORED, sl, false, of);
        if (is_long)
        {
            std::cout << std::endl;
        }
    }
}
