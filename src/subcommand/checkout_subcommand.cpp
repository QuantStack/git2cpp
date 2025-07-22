#include <iostream>
#include <sstream>

#include "../subcommand/checkout_subcommand.hpp"
#include "../utils/git_exception.hpp"
#include "../wrapper/repository_wrapper.hpp"

checkout_subcommand::checkout_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("checkout", "Switch branches or restore working tree files");

    sub->add_option("<branch>", m_branch_name, "Branch to checkout");
    sub->add_flag("-b", m_create_flag, "Create a new branch before checking it out");
    sub->add_flag("-B", m_force_create_flag, "Create a new branch or reset it if it exists before checking it out");
    sub->add_flag("-f, --force", m_force_checkout_flag, "When switching branches, proceed even if the index or the working tree differs from HEAD, and even if there are untracked files in the way");

    sub->callback([this]() { this->run(); });
}

void checkout_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    if (repo.state() != GIT_REPOSITORY_STATE_NONE)
    {
        throw std::runtime_error("Cannot checkout, repository is in unexpected state");
    }

    git_checkout_options options;
    git_checkout_options_init(&options, GIT_CHECKOUT_OPTIONS_VERSION);

    if(m_force_checkout_flag)
    {
        options.checkout_strategy = GIT_CHECKOUT_FORCE;
    }

    if (m_create_flag || m_force_create_flag)
    {
        auto annotated_commit = create_local_branch(repo, m_branch_name, m_force_create_flag);
        checkout_tree(repo, annotated_commit, m_branch_name, options);
        update_head(repo, annotated_commit, m_branch_name);
    }
    else
    {
        auto optional_commit = resolve_local_ref(repo, m_branch_name);
        if (!optional_commit)
        {
            // TODO: handle remote refs
            std::ostringstream buffer;
            buffer << "error: could not resolve pathspec '" << m_branch_name << "'" << std::endl;
            throw std::runtime_error(buffer.str());
        }
        checkout_tree(repo, *optional_commit, m_branch_name, options);
        update_head(repo, *optional_commit, m_branch_name);
    }
}

std::optional<annotated_commit_wrapper> checkout_subcommand::resolve_local_ref
(
    const repository_wrapper& repo,
    const std::string& target_name
)
{
    if (auto ref = repo.find_reference_dwim(target_name))
    {
        return repo.find_annotated_commit(*ref);
    }
    else if (auto obj = repo.revparse_single(target_name))
    {
        return repo.find_annotated_commit(obj->oid());
    }
    else
    {
        return std::nullopt;
    }
}

annotated_commit_wrapper checkout_subcommand::create_local_branch
(
    repository_wrapper& repo,
    const std::string& target_name,
    bool force
)
{
    auto branch = repo.create_branch(target_name, force);
    return repo.find_annotated_commit(branch);
}

void checkout_subcommand::checkout_tree
(
    const repository_wrapper& repo,
    const annotated_commit_wrapper& target_annotated_commit,
    const std::string& target_name,
    const git_checkout_options& options
)
{
    auto target_commit = repo.find_commit(target_annotated_commit.oid());
    throw_if_error(git_checkout_tree(repo, target_commit, &options));
}

void checkout_subcommand::update_head
(
    repository_wrapper& repo,
    const annotated_commit_wrapper& target_annotated_commit,
    const std::string& target_name
)
{
    std::string_view annotated_ref = target_annotated_commit.reference_name();
    if (!annotated_ref.empty())
    {
        auto ref = repo.find_reference(annotated_ref);
        if (ref.is_remote())
        {
            auto branch = repo.create_branch(target_name, target_annotated_commit);
            repo.set_head(branch.reference_name());
        }
        else
        {
            repo.set_head(annotated_ref);
        }
    }
    else
    {
        repo.set_head_detached(target_annotated_commit);
    }
}
