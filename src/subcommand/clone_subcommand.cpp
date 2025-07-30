#include <iostream>

#include "../subcommand/clone_subcommand.hpp"
#include "../utils/output.hpp"
#include "../wrapper/repository_wrapper.hpp"

clone_subcommand::clone_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("clone", "Clone a directory into a new repository");

    sub->add_option("<repository>", m_repository, "The (possibly remote) repository to clone from.")->required();
    sub->add_option("<directory>", m_directory, "The name of a new directory to clone into.");

    sub->callback([this]() { this->run(); });
}

namespace
{
    int sideband_progress(const char* str, int len, void*)
    {
        printf("remote: %.*s", len, str);
        fflush(stdout);
        return 0;
    }

    int fetch_progress(const git_indexer_progress* stats, void* payload)
    {
        static bool done = false;

        // We need to copy stats into payload even if the fetch is done,
        // because the checkout_progress callback will be called with the
        // same payload and needs the data to be up do date.
        auto* pr = reinterpret_cast<git_indexer_progress*>(payload);
        *pr = *stats;

        if (done)
        {
            return 0;
        }
        
        int network_percent = pr->total_objects > 0 ?
            (100 * pr->received_objects / pr->total_objects)
            : 0;
        size_t mbytes = pr->received_bytes / (1024*1024);

        std::cout << "Receiving objects: " << std::setw(4) << network_percent
            << "% (" << pr->received_objects << "/" << pr->total_objects << "), "
            << mbytes << " MiB";

        if (pr->received_objects == pr->total_objects)
        {
            std::cout << ", done." << std::endl;
            done = true;
        }
        else
        {
            std::cout << '\r';
        }
        return 0;
    }

    void checkout_progress(const char* path, size_t cur, size_t tot, void* payload)
    {
        static bool done = false;
        if (done)
        {
            return;
        }
        auto* pr = reinterpret_cast<git_indexer_progress*>(payload);
        int deltas_percent = pr->total_deltas > 0 ?
            (100 * pr->indexed_deltas / pr->total_deltas)
            : 0;

        std::cout << "Resolving deltas: " << std::setw(4) << deltas_percent
            << "% (" << pr->indexed_deltas << "/" << pr->total_deltas << ")";
        if (pr->indexed_deltas == pr->total_deltas)
        {
            std::cout << ", done." << std::endl;
            done = true;
        }
        else
        {
            std::cout << '\r';
        }
    }
}

void clone_subcommand::run()
{
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
