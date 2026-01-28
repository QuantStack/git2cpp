#include <iostream>
#include <git2.h>
#include <termcolor/termcolor.hpp>

#include "rebase_subcommand.hpp"
#include "../utils/git_exception.hpp"
#include "../wrapper/repository_wrapper.hpp"
#include "../wrapper/signature_wrapper.hpp"
#include "../wrapper/index_wrapper.hpp"

rebase_subcommand::rebase_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("rebase", "Reapply commits on top of another base tip");

    sub->add_option("upstream", m_upstream, "Upstream branch to rebase onto");
    sub->add_option("branch", m_branch, "Working branch; defaults to HEAD");
    sub->add_option("--onto", m_onto, "Starting point at which to create the new commits");

    sub->add_flag("--abort", m_abort, "Abort the rebase operation and reset HEAD to the original branch");
    sub->add_flag("--continue", m_continue, "Restart the rebasing process after having resolved a merge conflict");
    sub->add_flag("--skip", m_skip, "Restart the rebasing process by skipping the current patch");
    sub->add_flag("--quit", m_quit, "Abort the rebase operation but HEAD is not reset back to the original branch");

    sub->callback([this]() { this->run(); });
}

void ensure_rebase_in_progress(git_repository_state_t state)
{
    if (state != GIT_REPOSITORY_STATE_REBASE_INTERACTIVE && 
        state != GIT_REPOSITORY_STATE_REBASE_MERGE)
    {
        throw std::runtime_error("No rebase in progress");
    }
}

void rebase_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    git_repository_state_t state = repo.state();

    if (m_abort)
    {
        ensure_rebase_in_progress(state);
        run_abort(repo);
        return;
    }

    if (m_continue)
    {
        ensure_rebase_in_progress(state);
        run_continue(repo);
        return;
    }

    if (m_skip)
    {
        ensure_rebase_in_progress(state);
        run_skip(repo);
        return;
    }

    if (m_quit)
    {
        ensure_rebase_in_progress(state);
        run_quit(repo);
        return;
    }

    if (state == GIT_REPOSITORY_STATE_REBASE_INTERACTIVE || 
        state == GIT_REPOSITORY_STATE_REBASE_MERGE)
    {
        throw std::runtime_error("A rebase is already in progress");
    }

    if (state != GIT_REPOSITORY_STATE_NONE)
    {
        throw std::runtime_error("Cannot rebase:  repository is in unexpected state");
    }

    run_rebase(repo);
}

annotated_commit_wrapper rebase_subcommand::resolve_ref(
    const repository_wrapper& repo, 
    const std::string& ref_name) const
{
    if (!ref_name.empty())
    {
        auto branch_commit = repo.resolve_local_ref(m_branch);
        if (!branch_commit)
        {
            throw std::runtime_error("error: could not resolve branch '" + m_branch + "'");
        }
        return std::move(branch_commit).value();
    }
    else
    {
        auto head = repo.head();
        auto branch_commit = repo.find_annotated_commit(head);
        return branch_commit;
    }
}

void rebase_subcommand::perform_rebase(repository_wrapper& repo, rebase_wrapper& rebase)
{
    auto signatures = signature_wrapper::get_default_signature_from_env(repo);

    size_t total_operations = rebase.operation_entry_count();
    std::cout << "Rebasing " << total_operations << " commit(s)..." << std::endl;

    while (rebase.next_operation())
    {
        size_t current_idx = rebase.current_operation_index();

        commit_wrapper original_commit = repo.find_commit(rebase.current_operation_id());
        std::string commit_summary = original_commit.summary();

        std::cout << "Applying:  " << commit_summary << " (" 
                    << (current_idx + 1) << "/" << total_operations << ")" << std::endl;

        index_wrapper index = repo.make_index();
        if (index.has_conflict())
        {
            std:: cout << termcolor::red << "Conflicts detected!" << termcolor::reset << std::endl;
            std::cout << "Resolve conflicts and run:" << std::endl;
            std::cout << "  git2cpp rebase --continue" << std::endl;
            std::cout << "or skip this commit with:" << std::endl;
            std::cout << "  git2cpp rebase --skip" << std:: endl;
            std::cout << "or abort the rebase with:" << std:: endl;
            std::cout << "  git2cpp rebase --abort" << std::endl;
            return;
        }

        int commit_result = rebase.commit(signatures.first, signatures.second);

        if (commit_result == GIT_EAPPLIED)
        {
            std::cout << termcolor::yellow << "Skipping commit (already applied)" 
                        << termcolor::reset << std::endl;
        }
        else
        {
            throw_if_error(commit_result);
        }
    }

    rebase.finish(signatures.second);

    std::cout << termcolor::green << "Successfully rebased and updated HEAD." 
                << termcolor::reset << std::endl;
}

void rebase_subcommand::run_rebase(repository_wrapper& repo)
{
    if (m_upstream.empty())
    {
        throw std::runtime_error("upstream is required for rebase");
    }

    annotated_commit_wrapper branch = resolve_ref(repo, m_branch);
    std::optional<annotated_commit_wrapper> upstream = repo.resolve_local_ref(m_upstream);
    if (!upstream)
    {
        throw std::runtime_error("error: could not resolve upstream '" + m_upstream + "'");
    }

    std::unique_ptr<annotated_commit_wrapper> onto_ptr = nullptr;
    if (!m_onto.empty())
    {
        onto_ptr = std::make_unique<annotated_commit_wrapper>(resolve_ref(repo, m_onto));
    }
    git_rebase_options rebase_opts ;
    throw_if_error(git_rebase_options_init(&rebase_opts, GIT_REBASE_OPTIONS_VERSION));

    auto rebase = rebase_wrapper::init(repo, branch, upstream.value(), onto_ptr.get(), rebase_opts);
    perform_rebase(repo, rebase);
}

void rebase_subcommand::run_abort(repository_wrapper& repo)
{
    git_rebase_options rebase_opts;
    throw_if_error(git_rebase_options_init(&rebase_opts, GIT_REBASE_OPTIONS_VERSION));
    auto rebase = rebase_wrapper::open(repo, rebase_opts);
    rebase.abort();
    std::cout << "Rebase aborted" << std::endl;
}

void rebase_subcommand::run_continue(repository_wrapper& repo)
{
    // Check if there are still conflicts
    index_wrapper index = repo.make_index();
    if (index.has_conflict())
    {
        throw std::runtime_error("You must resolve conflicts before continuing the rebase");
    }
    git_rebase_options rebase_opts;
    throw_if_error(git_rebase_options_init(&rebase_opts, GIT_REBASE_OPTIONS_VERSION));
    auto rebase = rebase_wrapper::open(repo, rebase_opts);

    // Get signature for commits
    auto signatures = signature_wrapper::get_default_signature_from_env(repo);

    int commit_result = rebase.commit(signatures.first, signatures.second);

    if (commit_result == GIT_EAPPLIED)
    {
        std::cout << termcolor::yellow << "Skipping commit (already applied)" 
                    << termcolor::reset << std::endl;
    }
    else
    {
        throw_if_error(commit_result);
    }

    perform_rebase(repo, rebase);
}

void rebase_subcommand::run_skip(repository_wrapper& repo)
{
    git_rebase_options rebase_opts;
    throw_if_error(git_rebase_options_init(&rebase_opts, GIT_REBASE_OPTIONS_VERSION));
    auto rebase = rebase_wrapper::open(repo, rebase_opts);

    std::cout << "Skipping current commit..." << std::endl;

    perform_rebase(repo, rebase);
}

void rebase_subcommand:: run_quit(repository_wrapper& repo)
{
    repo.state_cleanup();
    std::cout << "Rebase state cleaned up (HEAD not reset)" << std::endl;
}

