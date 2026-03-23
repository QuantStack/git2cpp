#include "add_subcommand.hpp"

#include <git2.h>

#include "../wrapper/index_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"

add_subcommand::add_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("add", "Add file contents to the index");

    sub->add_option("<files>", m_add_files, "Files to add content from.");

    sub->add_flag("-A,--all,--no-ignore-removal", m_all_flag, "Update the index not only where the working tree has a file matching <pathspec> but also where the index already has an entry. This adds, modifies, and removes index entries to match the working tree.\n\nIf no <pathspec> is given when -A option is used, all files in the entire working tree are updated (old versions of Git used to limit the update to the current directory and its subdirectories).");
    // sub->add_flag("-n,--dryrun", dryrun_flag, "");
    // sub->add_flag("-u,--update", update_flag, "");
    // sub->add_flag("-v,--verbose", verbose_flag, "");

    sub->callback(
        [this]()
        {
            this->run();
        }
    );
};

void add_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    index_wrapper index = repo.make_index();

    if (m_all_flag)
    {
        index.add_all();
        index.write();
    }
    else
    {
        index.add_entries(m_add_files);
        index.write();
    }
}
