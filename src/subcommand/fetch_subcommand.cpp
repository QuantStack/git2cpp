#include <iostream>

#include <git2/remote.h>

#include "../subcommand/fetch_subcommand.hpp"
#include "../utils/output.hpp"
#include "../utils/progress.hpp"
#include "../wrapper/repository_wrapper.hpp"

fetch_subcommand::fetch_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("fetch", "Download objects and refs from another repository");

    sub->add_option("<remote>", m_remote_name, "The remote to fetch from")
        ->default_val("origin");
    sub->add_option("--depth", m_depth, "deepen or shorten history of shallow clone");
    // sub->add_option("--deepen", m_deepen, "deepen history of shallow clone");
    // sub->add_option("--shallow-since", m_shallow_since, "<time>\ndeepen history of shallow repository based on time.");
    // sub->add_option("--shallow-exclude", m_shallow_exclude, "<ref>\ndeepen history of shallow clone, excluding ref");
    sub->add_flag("--unshallow", m_unshallow, "convert to a complete repository");
    // sub->add_flag("--update-shallow", m_update_shallow, "accept refs that update .git/shallow");

    sub->callback([this]() { this->run(); });
}

void fetch_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    // Find the remote (default to origin if not specified)
    std::string remote_name = m_remote_name.empty() ? "origin" : m_remote_name;
    auto remote = repo.find_remote(remote_name);

    git_indexer_progress pd = {0};
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    fetch_opts.callbacks.sideband_progress = sideband_progress;
    fetch_opts.callbacks.transfer_progress = fetch_progress;
    fetch_opts.callbacks.payload = &pd;
    fetch_opts.callbacks.update_refs = update_refs;

    if (repo.is_shallow())
    {
        if (m_unshallow)
        {
            fetch_opts.depth = GIT_FETCH_DEPTH_UNSHALLOW;
        }
        else
        {
            fetch_opts.depth = m_depth;
        }
    }
    else
    {
        fetch_opts.depth = 0;
    }


    cursor_hider ch;

    // Perform the fetch
    remote.fetch(nullptr, &fetch_opts, "fetch");

    // Show statistics
    const git_indexer_progress* stats = git_remote_stats(remote);
    if (stats->local_objects > 0)
    {
        std::cout << "\rReceived " << stats->indexed_objects << "/" << stats->total_objects
            << " objects in " << stats->received_bytes << " bytes (used "
            << stats->local_objects << " local objects)" << std::endl;
    }
    else
    {
        std::cout << "\rReceived " << stats->indexed_objects << "/" << stats->total_objects
            << " objects in " << stats->received_bytes << " bytes" << std::endl;
    }
}
