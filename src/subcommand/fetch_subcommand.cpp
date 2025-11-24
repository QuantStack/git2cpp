#include <iostream>
#include <iomanip>

#include <git2/remote.h>

#include "../subcommand/fetch_subcommand.hpp"
#include "../wrapper/repository_wrapper.hpp"
#include "../wrapper/remote_wrapper.hpp"
#include "../utils/output.hpp"
#include "../utils/git_exception.hpp"

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

    int update_refs(const char* refname, const git_oid* a, const git_oid* b, git_refspec*, void*)
    {
        char a_str[GIT_OID_SHA1_HEXSIZE+1], b_str[GIT_OID_SHA1_HEXSIZE+1];
        
        git_oid_fmt(b_str, b);
        b_str[GIT_OID_SHA1_HEXSIZE] = '\0';

        if (git_oid_is_zero(a))
        {
            printf("[new]     %.20s %s\n", b_str, refname);
        }
        else
        {
            git_oid_fmt(a_str, a);
            a_str[GIT_OID_SHA1_HEXSIZE] = '\0';
            printf("[updated] %.10s..%.10s %s\n", a_str, b_str, refname);
        }

        return 0;
    }
}

fetch_subcommand::fetch_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("fetch", "Download objects and refs from another repository");

    sub->add_option("<remote>", m_remote_name, "The remote to fetch from")
        ->default_val("origin");

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

    cursor_hider ch;
    
    // Perform the fetch
    throw_if_error(git_remote_fetch(remote, nullptr, &fetch_opts, "fetch"));

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

