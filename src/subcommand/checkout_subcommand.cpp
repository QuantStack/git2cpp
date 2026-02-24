#include <iostream>
#include <sstream>
#include <set>

#include "../subcommand/checkout_subcommand.hpp"
#include "../subcommand/status_subcommand.hpp"
#include "../utils/git_exception.hpp"
#include "../wrapper/repository_wrapper.hpp"
#include "../wrapper/status_wrapper.hpp"

checkout_subcommand::checkout_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("checkout", "Switch branches or restore working tree files");

    sub->add_option("<branch>", m_branch_name, "Branch to checkout");
    sub->add_flag("-b", m_create_flag, "Create a new branch before checking it out");
    sub->add_flag("-B", m_force_create_flag, "Create a new branch or reset it if it exists before checking it out");
    sub->add_flag("-f, --force", m_force_checkout_flag, "When switching branches, proceed even if the index or the working tree differs from HEAD, and even if there are untracked files in the way");

    sub->callback([this]() { this->run(); });
}

void print_no_switch(status_list_wrapper& sl)
{
    std::cout << "Your local changes to the following files would be overwritten by checkout:" << std::endl;

    for (const auto* entry : sl.get_entry_list(GIT_STATUS_WT_MODIFIED))
    {
        std::cout << "\t" << entry->index_to_workdir->new_file.path << std::endl;
    }
    for (const auto* entry : sl.get_entry_list(GIT_STATUS_WT_DELETED))
    {
        std::cout << "\t" << entry->index_to_workdir->old_file.path << std::endl;
    }

    std::cout << "Please commit your changes or stash them before you switch branches.\nAborting" << std::endl;
    return;
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

    if (m_force_checkout_flag)
    {
        options.checkout_strategy = GIT_CHECKOUT_FORCE;
    }
    else
    {
        options.checkout_strategy = GIT_CHECKOUT_SAFE;
    }

    if (m_create_flag || m_force_create_flag)
    {
        auto annotated_commit = create_local_branch(repo, m_branch_name, m_force_create_flag);
        checkout_tree(repo, annotated_commit, m_branch_name, options);
        update_head(repo, annotated_commit, m_branch_name);

        std::cout << "Switched to a new branch '" << m_branch_name << "'" << std::endl;
    }
    else
    {
        auto optional_commit = repo.resolve_local_ref(m_branch_name);
        if (!optional_commit)
        {
            // TODO: handle remote refs
            std::ostringstream buffer;
            buffer << "error: could not resolve pathspec '" << m_branch_name << "'" << std::endl;
            throw std::runtime_error(buffer.str());
        }

        auto sl = status_list_wrapper::status_list(repo);
        try
        {
            checkout_tree(repo, *optional_commit, m_branch_name, options);
            update_head(repo, *optional_commit, m_branch_name);
        }
        catch (const git_exception& e)
        {
            if (sl.has_notstagged_header())
            {
                print_no_switch(sl);
            }
            throw e;
        }

        if (sl.has_notstagged_header())
        {
            bool is_long = false;
            bool is_coloured = false;
            std::set<std::string> tracked_dir_set{};
            print_notstagged(sl, tracked_dir_set, is_long, is_coloured);
        }
        if (sl.has_tobecommited_header())
        {
            bool is_long = false;
            bool is_coloured = false;
            std::set<std::string> tracked_dir_set{};
            print_tobecommited(sl, tracked_dir_set, is_long, is_coloured);
        }
        std::cout << "Switched to branch '" << m_branch_name << "'" << std::endl;
        print_tracking_info(repo, sl, true);
    }
}

annotated_commit_wrapper checkout_subcommand::create_local_branch
(
    repository_wrapper& repo,
    const std::string_view target_name,
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
    const std::string_view target_name,
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
    const std::string_view target_name
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
