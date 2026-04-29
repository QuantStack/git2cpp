#include "status_subcommand.hpp"

#include <algorithm>
#include <iostream>
#include <ostream>
#include <set>
#include <string>
#include <unordered_map>

#include <git2.h>
#include <termcolor/termcolor.hpp>

status_subcommand::status_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand(
        "status",
        "Show modified files in working directory, staged for your next commit"
    );

    sub->add_flag("-s,--short", m_options.m_short_flag, "Give the output in the short-format.");
    sub->add_flag("--long", m_options.m_long_flag, "Give the output in the long-format. This is the default.");
    // sub->add_flag("--porcelain[=<version>]", porcelain, "Give the output in an easy-to-parse format for
    // scripts.
    //     This is similar to the short output, but will remain stable across Git versions and regardless of
    //     user configuration. See below for details. The version parameter is used to specify the format
    //     version. This is optional and defaults to the original version v1 format.");
    sub->add_flag("-b,--branch", m_options.m_branch_flag, "Show the branch and tracking info even in short-format.");

    sub->callback(
        [this]()
        {
            this->run();
        }
    );
};

namespace
{
    const std::string
        untracked_header = "Untracked files:\n  (use \"git add <file>...\" to include in what will be committed)\n";
    const std::string tobecommited_header = "Changes to be committed:\n  (use \"git reset HEAD <file>...\" to unstage)\n";
    const std::string
        ignored_header = "Ignored files:\n  (use \"git add -f <file>...\" to include in what will be committed)\n";
    const std::string
        notstagged_header = "Changes not staged for commit:\n  (use \"git add <file>...\" to update what will be committed)\n";
    // TODO: add the following ot notstagged_header after "checkout <file>" is implemented: (use \"git
    // checkout -- <file>...\" to discard changes in working directory)\n";
    const std::string unmerged_header = "Unmerged paths:\n  (use \"git add <file>...\" to mark resolution)\n";
    const std::string nothingtocommit_message = "no changes added to commit  (use \"git add\" and/or \"git commit -a\")";
    const std::string treeclean_message = "Nothing to commit, working tree clean";

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

    struct combined_entry
    {
        git_status_t index_status = GIT_STATUS_CURRENT;
        git_status_t workdir_status = GIT_STATUS_CURRENT;
        std::string item;
    };

    std::string get_print_status(git_status_t status, bool is_long)
    {
        std::string entry_status;
        if (is_long)
        {
            entry_status = get_status_msg(status).long_mod;
        }
        else
        {
            entry_status = get_status_msg(status).short_mod;
        }
        return entry_status;
    }

    char short_char_from_status(git_status_t status)
    {
        switch (status)
        {
            case GIT_STATUS_INDEX_NEW:
            case GIT_STATUS_WT_NEW:
                return 'A';
            case GIT_STATUS_INDEX_MODIFIED:
            case GIT_STATUS_WT_MODIFIED:
                return 'M';
            case GIT_STATUS_INDEX_DELETED:
            case GIT_STATUS_WT_DELETED:
                return 'D';
            case GIT_STATUS_INDEX_RENAMED:
            case GIT_STATUS_WT_RENAMED:
                return 'R';
            case GIT_STATUS_INDEX_TYPECHANGE:
            case GIT_STATUS_WT_TYPECHANGE:
                return 'T';
            default:
                return ' ';
        }
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

    std::unordered_map<std::string, combined_entry>
    build_combined_status_map(status_list_wrapper& sl, std::set<std::string>& tracked_dir_set)
    {
        std::unordered_map<std::string, combined_entry> combined;

        auto update_status_map =
            [&sl, &tracked_dir_set, &combined](const git_status_t(&status_array)[5], bool index)
        {
            for (git_status_t status : status_array)
            {
                const auto& list = sl.get_entry_list(status);
                for (auto* entry : list)
                {
                    git_diff_delta* dd = index ? entry->head_to_index : entry->index_to_workdir;
                    const char* old_path = dd->old_file.path;
                    const char* new_path = dd->new_file.path;
                    update_tracked_dir_set(old_path, &tracked_dir_set);
                    std::string item = get_print_item(old_path, new_path);
                    auto& ce = combined[item];
                    ce.item = item;
                    if (index)
                    {
                        ce.index_status = status;
                    }
                    else
                    {
                        ce.workdir_status = status;
                    }
                }
            }
        };

        const git_status_t index_statuses[] = {
            GIT_STATUS_INDEX_NEW,
            GIT_STATUS_INDEX_MODIFIED,
            GIT_STATUS_INDEX_DELETED,
            GIT_STATUS_INDEX_RENAMED,
            GIT_STATUS_INDEX_TYPECHANGE
        };
        update_status_map(index_statuses, true);

        const git_status_t worktree_statuses[] = {
            GIT_STATUS_WT_NEW,
            GIT_STATUS_WT_MODIFIED,
            GIT_STATUS_WT_DELETED,
            GIT_STATUS_WT_TYPECHANGE,
            GIT_STATUS_WT_RENAMED
        };
        update_status_map(worktree_statuses, false);

        return combined;
    }

    void print_combined_short(const std::unordered_map<std::string, combined_entry>& entries_map, bool is_coloured)
    {
        std::vector<std::string> keys;
        keys.reserve(entries_map.size());
        for (const auto& kv : entries_map)
        {
            keys.push_back(kv.first);
        }
        std::sort(keys.begin(), keys.end());

        struct normal_row
        {
            char idx;
            char wt;
            std::string item;
        };

        std::vector<normal_row> normal_rows;
        std::vector<std::string> untracked_items;
        std::vector<std::string> ignored_items;

        for (const auto& k : keys)
        {
            const auto& ce = entries_map.at(k);

            // Collect special cases to print last (untracked, ignored) only when not staged.
            if ((ce.workdir_status & GIT_STATUS_WT_NEW) && ce.index_status == 0)
            {
                untracked_items.push_back(ce.item);
                continue;
            }
            if ((ce.workdir_status & GIT_STATUS_IGNORED || ce.index_status & GIT_STATUS_IGNORED)
                && ce.index_status == 0)
            {
                ignored_items.push_back(ce.item);
                continue;
            }

            // Regular two-column entry (may include index or worktree or both)
            char idx = short_char_from_status(ce.index_status);
            char wt = short_char_from_status(ce.workdir_status);
            normal_rows.push_back({idx, wt, ce.item});
        }

        for (const auto& r : normal_rows)
        {
            if (is_coloured)
            {
                std::cout << termcolor::green << r.idx << termcolor::reset;
                std::cout << termcolor::red << r.wt << termcolor::reset;
                std::cout << " " << r.item << std::endl;
            }
            else
            {
                std::cout << r.idx << r.wt << " " << r.item << std::endl;
            }
        }

        stream_colour_fn colour;
        colour = is_coloured ? termcolor::red : termcolor::bright_white;

        std::sort(untracked_items.begin(), untracked_items.end());
        for (const auto& it : untracked_items)
        {
            std::cout << colour << "?? " << termcolor::reset << it << std::endl;
        }

        std::sort(ignored_items.begin(), ignored_items.end());
        for (const auto& it : ignored_items)
        {
            std::cout << colour << "!! " << termcolor::reset << it << std::endl;
        }
    }

    std::vector<print_entry> get_entries_to_print(
        git_status_t status,
        status_list_wrapper& sl,
        bool head_selector,
        bool is_long,
        std::set<std::string>* tracked_dir_set = nullptr
    )
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

            print_entry e = {get_print_status(status, is_long), get_print_item(old_path, new_path)};

            entries_to_print.push_back(std::move(e));
        }
        return entries_to_print;
    }

    void print_entries(std::vector<print_entry> entries_to_print, bool is_long, stream_colour_fn colour)
    {
        for (const auto& e : entries_to_print)
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

    void print_not_tracked(
        const std::vector<print_entry>& entries_to_print,
        const std::set<std::string>& tracked_dir_set,
        std::set<std::string>& untracked_dir_set,
        bool is_long,
        stream_colour_fn colour
    )
    {
        std::vector<print_entry> not_tracked_entries_to_print{};
        for (const auto& e : entries_to_print)
        {
            const size_t first_slash_idx = e.item.find('/');
            if (std::string::npos != first_slash_idx)
            {
                auto directory = e.item.substr(0, first_slash_idx) + "/";
                if (tracked_dir_set.contains(directory))
                {
                    not_tracked_entries_to_print.push_back(e);
                }
                else
                {
                    if (!untracked_dir_set.contains(directory))
                    {
                        not_tracked_entries_to_print.push_back({e.status, directory});
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
}

void print_tracking_info(repository_wrapper& repo, status_list_wrapper& sl, bool is_long, bool branch_flag)
{
    auto tracking_info = repo.get_tracking_info();

    if (is_long)
    {
        if (tracking_info.has_upstream)
        {
            if (tracking_info.ahead > 0 && tracking_info.behind == 0)
            {
                std::cout << "Your branch is ahead of '" << tracking_info.upstream_name << "' by "
                          << tracking_info.ahead << " commit" << (tracking_info.ahead > 1 ? "s" : "") << "."
                          << std::endl;
                std::cout << "  (use \"git push\" to publish your local commits)" << std::endl;
            }
            else if (tracking_info.ahead == 0 && tracking_info.behind > 0)
            {
                std::cout << "Your branch is behind '" << tracking_info.upstream_name << "' by "
                          << tracking_info.behind << " commit" << (tracking_info.behind > 1 ? "s" : "") << "."
                          << std::endl;
                std::cout << "  (use \"git pull\" to update your local branch)" << std::endl;
            }
            else if (tracking_info.ahead > 0 && tracking_info.behind > 0)
            {
                std::cout << "Your branch and '" << tracking_info.upstream_name << "' have diverged,"
                          << std::endl;
                std::cout << "and have " << tracking_info.ahead << " and " << tracking_info.behind
                          << " different commit"
                          << ((tracking_info.ahead + tracking_info.behind) > 2 ? "s" : "")
                          << " each, respectively." << std::endl;
                std::cout << "  (use \"git pull\" to merge the remote branch into yours)" << std::endl;
            }
            else  // ahead == 0 && behind == 0
            {
                std::cout << "Your branch is up to date with '" << tracking_info.upstream_name << "'."
                          << std::endl;
            }
            std::cout << std::endl;
        }

        if (repo.is_head_unborn())
        {
            std::cout << "No commit yet\n" << std::endl;
        }

        if (sl.has_unmerged_header())
        {
            std::cout << "You have unmerged paths.\n  (fix conflicts and run \"git commit\")\n  (use \"git merge --abort\" to abort the merge)\n"
                      << std::endl;
        }
    }
    else if (branch_flag)
    {
        if (tracking_info.has_upstream)
        {
            std::cout << "..." << termcolor::red << tracking_info.upstream_name << termcolor::reset;

            if (tracking_info.ahead > 0 || tracking_info.behind > 0)
            {
                std::cout << " [";
                if (tracking_info.ahead > 0)
                {
                    std::cout << "ahead " << termcolor::green << tracking_info.ahead << termcolor::reset;
                }
                if (tracking_info.behind > 0)
                {
                    if (tracking_info.ahead > 0)
                    {
                        std::cout << ", ";
                    }
                    std::cout << "behind " << termcolor::red << tracking_info.behind << termcolor::reset;
                }
                std::cout << "]";
            }
            std::cout << std::endl;
        }
    }
}

void print_tobecommited(status_list_wrapper& sl, std::set<std::string> tracked_dir_set, bool is_long, bool is_coloured)
{
    stream_colour_fn colour;
    colour = is_coloured ? termcolor::green : termcolor::bright_white;

    if (is_long)
    {
        std::cout << tobecommited_header;
    }
    print_entries(get_entries_to_print(GIT_STATUS_INDEX_NEW, sl, true, is_long, &tracked_dir_set), is_long, colour);
    print_entries(
        get_entries_to_print(GIT_STATUS_INDEX_MODIFIED, sl, true, is_long, &tracked_dir_set),
        is_long,
        colour
    );
    print_entries(
        get_entries_to_print(GIT_STATUS_INDEX_DELETED, sl, true, is_long, &tracked_dir_set),
        is_long,
        colour
    );
    print_entries(
        get_entries_to_print(GIT_STATUS_INDEX_RENAMED, sl, true, is_long, &tracked_dir_set),
        is_long,
        colour
    );
    print_entries(
        get_entries_to_print(GIT_STATUS_INDEX_TYPECHANGE, sl, true, is_long, &tracked_dir_set),
        is_long,
        colour
    );
    if (is_long)
    {
        std::cout << std::endl;
    }
}

void print_notstagged(status_list_wrapper& sl, std::set<std::string> tracked_dir_set, bool is_long, bool is_coloured)
{
    stream_colour_fn colour;
    colour = is_coloured ? termcolor::red : termcolor::bright_white;

    if (is_long)
    {
        std::cout << notstagged_header;
    }
    print_entries(
        get_entries_to_print(GIT_STATUS_WT_MODIFIED, sl, false, is_long, &tracked_dir_set),
        is_long,
        colour
    );
    print_entries(
        get_entries_to_print(GIT_STATUS_WT_DELETED, sl, false, is_long, &tracked_dir_set),
        is_long,
        colour
    );
    print_entries(
        get_entries_to_print(GIT_STATUS_WT_TYPECHANGE, sl, false, is_long, &tracked_dir_set),
        is_long,
        colour
    );
    print_entries(
        get_entries_to_print(GIT_STATUS_WT_RENAMED, sl, false, is_long, &tracked_dir_set),
        is_long,
        colour
    );
    if (is_long)
    {
        std::cout << std::endl;
    }
}

void print_unmerged(
    status_list_wrapper& sl,
    std::set<std::string> tracked_dir_set,
    std::set<std::string> untracked_dir_set,
    bool is_long,
    bool is_coloured
)
{
    stream_colour_fn colour;
    colour = is_coloured ? termcolor::red : termcolor::bright_white;

    if (is_long)
    {
        std::cout << unmerged_header;
    }
    print_not_tracked(
        get_entries_to_print(GIT_STATUS_CONFLICTED, sl, false, is_long),
        tracked_dir_set,
        untracked_dir_set,
        is_long,
        colour
    );
    if (is_long)
    {
        std::cout << std::endl;
    }
}

void print_untracked(
    status_list_wrapper& sl,
    std::set<std::string> tracked_dir_set,
    std::set<std::string> untracked_dir_set,
    bool is_long,
    bool is_coloured
)
{
    stream_colour_fn colour;
    if (is_coloured)
    {
        colour = termcolor::red;
    }
    else
    {
        colour = termcolor::bright_white;
    }

    if (is_long)
    {
        std::cout << untracked_header;
    }
    print_not_tracked(
        get_entries_to_print(GIT_STATUS_WT_NEW, sl, false, is_long),
        tracked_dir_set,
        untracked_dir_set,
        is_long,
        colour
    );
    if (is_long)
    {
        std::cout << std::endl;
    }
}

void status_subcommand::run()
{
    status_run(m_options);
}

void status_run(status_subcommand_options options)
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);
    auto sl = status_list_wrapper::status_list(repo);

    std::set<std::string> tracked_dir_set{};
    std::set<std::string> untracked_dir_set{};
    std::vector<std::string> untracked_to_print{};
    std::vector<std::string> ignored_to_print{};

    output_format of = output_format::DEFAULT;
    if (options.m_short_flag)
    {
        of = output_format::SHORT;
    }
    if (options.m_long_flag)
    {
        of = output_format::LONG;
    }
    // else if (porcelain_format)
    // {
    //     output_format = 3;
    // }

    bool is_long;
    is_long = ((of == output_format::DEFAULT) || (of == output_format::LONG));

    auto branch_name = repo.head_short_name();
    bool is_coloured = true;
    bool branch_flag = options.m_branch_flag;
    if (is_long)
    {
        std::cout << "On branch " << branch_name << std::endl;
    }
    else if (branch_flag)
    {
        if (is_coloured)
        {
            std::cout << "## " << termcolor::green << branch_name << termcolor::reset;
        }
        else
        {
            std::cout << "## " << branch_name;
        }
    }

    print_tracking_info(repo, sl, is_long, branch_flag);

    if (of == output_format::SHORT)
    {
        auto combined = build_combined_status_map(sl, tracked_dir_set);
        print_combined_short(combined, is_coloured);
        return;
    }

    if (sl.has_tobecommited_header())
    {
        print_tobecommited(sl, tracked_dir_set, is_long, is_coloured);
    }

    if (sl.has_unmerged_header())
    {
        print_unmerged(sl, tracked_dir_set, untracked_dir_set, is_long, is_coloured);
    }

    if (sl.has_notstagged_header())
    {
        print_notstagged(sl, tracked_dir_set, is_long, is_coloured);
    }

    if (sl.has_untracked_header())
    {
        print_untracked(sl, tracked_dir_set, untracked_dir_set, is_long, is_coloured);
    }

    // TODO: check if this message should be displayed even if there are untracked files
    if (is_long
        && !(
            sl.has_tobecommited_header() || sl.has_notstagged_header() || sl.has_unmerged_header()
            || sl.has_untracked_header()
        ))
    {
        std::cout << treeclean_message << std::endl;
    }

    if (is_long & !sl.has_tobecommited_header() && (sl.has_notstagged_header() || sl.has_untracked_header()))
    {
        std::cout << nothingtocommit_message << std::endl;
    }
}
