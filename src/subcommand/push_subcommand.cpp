#include <iostream>

#include <git2/remote.h>

#include "../subcommand/push_subcommand.hpp"
#include "../wrapper/repository_wrapper.hpp"
#include "../wrapper/remote_wrapper.hpp"
#include "../utils/git_exception.hpp"
#include "../utils/common.hpp"

namespace
{
    int push_transfer_progress(unsigned int current, unsigned int total, size_t bytes, void*)
    {
        if (total > 0)
        {
            int percent = (100 * current) / total;
            std::cout << "Writing objects: " << percent << "% (" << current 
                << "/" << total << "), " << bytes << " bytes\r";
        }
        return 0;
    }

    int push_update_reference(const char* refname, const char* status, void*)
    {
        if (status)
        {
            std::cout << "  " << refname << " " << status << std::endl;
        }
        else
        {
            std::cout << "  " << refname << std::endl;
        }
        return 0;
    }
}

push_subcommand::push_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("push", "Update remote refs along with associated objects");

    sub->add_option("<remote>", m_remote_name, "The remote to push to")
        ->default_val("origin");
    
    sub->add_option("<refspec>", m_refspecs, "The refspec(s) to push");

    sub->callback([this]() { this->run(); });
}

void push_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    std::string remote_name = m_remote_name.empty() ? "origin" : m_remote_name;
    auto remote = repo.find_remote(remote_name);

    git_push_options push_opts = GIT_PUSH_OPTIONS_INIT;
    push_opts.callbacks.push_transfer_progress = push_transfer_progress;
    push_opts.callbacks.push_update_reference = push_update_reference;

    git_strarray_wrapper refspecs_wrapper(m_refspecs);
    git_strarray* refspecs_ptr = nullptr;
    if (!m_refspecs.empty())
    {
        refspecs_ptr = refspecs_wrapper;
    }

    throw_if_error(git_remote_push(remote, refspecs_ptr, &push_opts));
    std::cout << "Pushed to " << remote_name << std::endl;
}

