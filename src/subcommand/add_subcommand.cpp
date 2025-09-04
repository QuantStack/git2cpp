#include <git2.h>

#include "add_subcommand.hpp"
#include "../wrapper/index_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"


add_subcommand::add_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("add", "Add file contents to the index");

    sub->add_option("files", m_add_files, "Files to add");

    sub->add_flag("-A,--all,--no-ignore-removal", m_all_flag, "");
    // sub->add_flag("-n,--dryrun", dryrun_flag, "");
    // sub->add_flag("-u,--update", update_flag, "");
    // sub->add_flag("-v,--verbose", verbose_flag, "");

    sub->callback([this]() { this->run(); });
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
