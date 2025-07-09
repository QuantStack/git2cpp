#include <git2.h>

#include "add_subcommand.hpp"
#include "../wrapper/index_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"


add_subcommand::add_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("add", "Add file contents to the index");

    sub->add_flag("-A,--all,--no-ignore-removal", all_flag, "");
    // sub->add_flag("-n,--dryrun", dryrun_flag, "");
    // sub->add_flag("-u,--update", update_flag, "");
    // sub->add_flag("-v,--verbose", verbose_flag, "");

    sub->callback([this]() { this->run(); });
};

void add_subcommand::run()
{
    auto directory = get_current_git_path();
    auto bare = false;
    auto repo = repository_wrapper::init(directory, bare);

    index_wrapper index = repo.get_index();

    if (all_flag)
    {
        index.add_all();
    }
    // else
    // {
    //     index.add_entry();
    // }
}
