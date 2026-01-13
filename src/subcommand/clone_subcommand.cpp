#include <iostream>

#include "../subcommand/clone_subcommand.hpp"
#include "../utils/output.hpp"
#include "../utils/progress.hpp"
#include "../wrapper/repository_wrapper.hpp"

clone_subcommand::clone_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("clone", "Clone a directory into a new repository");

    sub->add_option("<repository>", m_repository, "The (possibly remote) repository to clone from.")->required();
    sub->add_option("<directory>", m_directory, "The name of a new directory to clone into.");
    sub->add_option("--depth", m_depth, "Create a shallow clone of that depth.");
    // sub->add_option("--shallow-since", m_shallow_since, "<time>\ndeepen history of shallow repository based on time.");
    // sub->add_option("--shallow-exclude", m_shallow_exclude, "<ref>\ndeepen history of shallow clone, excluding ref");
    sub->add_flag("--bare", m_bare, "Create a bare Git repository.");

    sub->callback([this]() { this->run(); });
}

void clone_subcommand::run()
{
    // m_depth = 0 means no shallow clone in libgit2, while
    // it is forbidden with git. Therefore we use another
    // sentinel value to detect full clone.
    if (m_depth == 0)
    {
        std::cout << "fatal: depth 0 is not a positive number" << std::endl;
        return;
    }

    if (m_depth == std::numeric_limits<size_t>::max())
    {
        m_depth = 0;
    }

    git_indexer_progress pd;
    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_opts.progress_cb = checkout_progress;
    checkout_opts.progress_payload = &pd;
    clone_opts.checkout_opts = checkout_opts;
    clone_opts.fetch_opts.callbacks.sideband_progress = sideband_progress;
    clone_opts.fetch_opts.callbacks.transfer_progress = fetch_progress;
    clone_opts.fetch_opts.callbacks.payload = &pd;
    clone_opts.fetch_opts.depth = m_depth;
    clone_opts.bare = m_bare ? 1 : 0;

    std::string short_name = m_directory;
    if (m_directory.empty())
    {
        auto size = m_repository.size();
        auto begin = m_repository.find_last_of('/') + 1;
        auto end = m_repository.ends_with(".git") ? size - 4 : size;
        auto count = end - begin;
        short_name = m_repository.substr(begin, count);
        m_directory = get_current_git_path() + '/' + short_name;
    }
    std::cout << "Cloning into '" + short_name + "'..." << std::endl;
    cursor_hider ch;
    repository_wrapper::clone(m_repository, m_directory, clone_opts);
}
