#include <iostream>

#include <git2/remote.h>

#include "../subcommand/push_subcommand.hpp"
#include "../utils/progress.hpp"
#include "../wrapper/repository_wrapper.hpp"

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

    if (m_refspecs.empty())
    {
        try
        {
            auto head_ref = repo.head();
            std::string short_name = head_ref.short_name();
            std::string refspec = "refs/heads/" + short_name;
            m_refspecs.push_back(refspec);
        }
        catch (...)
        {
            std::cerr << "Could not determine current branch to push." << std::endl;
            return;
        }
    }
    git_strarray_wrapper refspecs_wrapper(m_refspecs);
    git_strarray* refspecs_ptr = nullptr;
    refspecs_ptr = refspecs_wrapper;

    remote.push(refspecs_ptr, &push_opts);
    std::cout << "Pushed to " << remote_name << std::endl;
}
