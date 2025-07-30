#include <git2.h>
#include <unistd.h>

#include "commit_subcommand.hpp"
#include "../wrapper/index_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"


commit_subcommand::commit_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("commit", "Record changes to the repository");

    sub->add_option("commit_message", m_commit_message, "Commit message");

    sub->add_flag("-m,--message", m_commit_message_flag, "");

    sub->callback([this]() { this->run(); });
};


void commit_subcommand::run()
{
    auto directory = get_current_git_path();
    auto bare = false;
    auto repo = repository_wrapper::init(directory, bare);
    auto author_committer_signatures =  signature_wrapper::get_default_signature_from_env(repo);

    if (!m_commit_message_flag)
    {
        throw std::runtime_error("Please provide a message using the -m flag.");
    }

    repo.create_commit(author_committer_signatures, m_commit_message);
}
