#include <iostream>

#include "../subcommand/branch_subcommand.hpp"
#include "../wrapper/repository_wrapper.hpp"

branch_subcommand::branch_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("branch", "List, create or delete branches");

    sub->add_option("<branchname>", m_branch_name, "The name of the branch to create or delete");

    sub->add_flag("-d,--delete", m_deletion_flag, "Delete a branch");
    sub->add_flag("-a,--all", m_all_flag, "List both remote-tracking branches and local branches");
    sub->add_flag("-r,--remotes", m_remote_flag, "List or delete (if used with -d) the remote-tracking branches");
    sub->add_flag("-l,--list", m_list_flag, "List branches");
    sub->add_flag("-f,--force", m_force_flag, "Skips confirmation");

    sub->callback([this]() { this->run(); });
}

void branch_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    if (m_list_flag || m_branch_name.empty())
    {
        auto head_name = repo.head().short_name();
        std::cout << "* " << head_name << std::endl;
        git_branch_t type = m_all_flag ? GIT_BRANCH_ALL : (m_remote_flag ? GIT_BRANCH_REMOTE : GIT_BRANCH_LOCAL);
        auto iter = repo.iterate_branches(type);
        auto br = iter.next();
        while (br)
        {
            if (br->name() != head_name)
            {
                std::cout << "  " << br->name() << std::endl;
            }
            br = iter.next();
        }
    }
    else if (m_deletion_flag)
    {
        run_deletion(repo);
    }
    else
    {
        run_creation(repo);
    }
}

void branch_subcommand::run_deletion(repository_wrapper& repo)
{
    auto branch = repo.find_branch(m_branch_name);
    // TODO: handle unmerged stated once we handle upstream repos
    delete_branch(std::move(branch));
}


void branch_subcommand::run_creation(repository_wrapper& repo)
{
    // TODO: handle specification of starting commit
    repo.create_branch(m_branch_name, m_force_flag);
}
