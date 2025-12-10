#include <iostream>
#include <stdexcept>

#include "../subcommand/remote_subcommand.hpp"
#include "../wrapper/repository_wrapper.hpp"

remote_subcommand::remote_subcommand(const libgit2_object&, CLI::App& app)
{
    m_subcommand = app.add_subcommand("remote", "Manage set of tracked repositories");

    m_subcommand->add_option("operation", m_operation, "Operation: add, remove, rename, set-url, show")
        ->check(CLI::IsMember({"add", "remove", "rm", "rename", "set-url", "show"}));

    m_subcommand->add_flag("-v,--verbose", m_verbose_flag, "Be verbose");
    m_subcommand->add_flag("--push", m_push_flag, "Set push URL instead of fetch URL");

    // Allow positional arguments after operation
    m_subcommand->allow_extras();

    m_subcommand->callback([this]() { this->run(); });
}

void remote_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    // Get extra positional arguments
    auto extras = m_subcommand->remaining();

    // Parse positional arguments based on operation
    if (m_operation == "add")
    {
        if (extras.size() == 2)
        {
            m_remote_name = extras[0];
            m_url = extras[1];
        }
        run_add(repo);
    }
    else if (m_operation == "remove" || m_operation == "rm")
    {
        if (extras.size() == 1)
        {
            m_remote_name = extras[0];
        }
        run_remove(repo);
    }
    else if (m_operation == "rename")
    {
        if (extras.size() == 2)
        {
            m_old_name = extras[0];
            m_new_name = extras[1];
        }
        run_rename(repo);
    }
    else if (m_operation == "set-url")
    {
        // Handle --push flag before arguments
        size_t arg_idx = 0;
        if (extras.size() > 0 && extras[0] == "--push")
        {
            m_push_flag = true;
            arg_idx = 1;
        }
        if (extras.size() >= arg_idx + 2)
        {
            m_remote_name = extras[arg_idx];
            m_new_name = extras[arg_idx + 1];
            run_seturl(repo);
        }
        else if (m_remote_name.empty() || m_new_name.empty())
        {
            throw std::runtime_error("remote set-url requires both name and new URL");
        }
        else
        {
            run_seturl(repo);
        }
    }
    else if (m_operation.empty() || m_operation == "show")
    {
        if (extras.size() >= 1)
        {
            m_remote_name = extras[0];
        }
        run_show(repo);
    }
}

void remote_subcommand::run_add(repository_wrapper& repo)
{
    if (m_remote_name.empty())
    {
        throw std::runtime_error("usage: git remote add <name> <url>");   // TODO: add [<options>] when implemented
    }
    repo.create_remote(m_remote_name, m_url);
}

void remote_subcommand::run_remove(repository_wrapper& repo)
{
    if (m_remote_name.empty())
    {
        throw std::runtime_error("usage: git remote remove <name>");
    }
    repo.delete_remote(m_remote_name);
}

void remote_subcommand::run_rename(repository_wrapper& repo)
{
    if (m_old_name.empty())
    {
        throw std::runtime_error("usage: git remote rename <old> <new>");   // TODO: add  [--[no-]progress] when implemented
    }
    repo.rename_remote(m_old_name, m_new_name);
}

void remote_subcommand::run_seturl(repository_wrapper& repo)
{
    if (m_remote_name.empty() || m_new_name.empty())
    {
        throw std::runtime_error("remote set-url requires both name and new URL");
    }
    repo.set_remote_url(m_remote_name, m_new_name, m_push_flag);
}

void remote_subcommand::run_show(const repository_wrapper& repo)
{
    auto remotes = repo.list_remotes();

    if (m_remote_name.empty())
    {
        // Show all remotes
        for (const auto& name : remotes)
        {
            if (m_verbose_flag)
            {
                auto remote = repo.find_remote(name);
                auto fetch_url = remote.url();
                auto push_url = remote.pushurl();

                if (!fetch_url.empty())
                {
                    std::cout << name << "\t" << fetch_url << " (fetch)" << std::endl;
                }
                if (!push_url.empty())
                {
                    std::cout << name << "\t" << push_url << " (push)" << std::endl;
                }
                else if (!fetch_url.empty())
                {
                    std::cout << name << "\t" << fetch_url << " (push)" << std::endl;
                }
            }
            else
            {
                std::cout << name << std::endl;
            }
        }
    }
    else
    {
        // Show specific remote
        auto remote = repo.find_remote(m_remote_name);
        std::cout << "* remote " << m_remote_name << std::endl;

        auto fetch_url = remote.url();
        if (!fetch_url.empty())
        {
            std::cout << "  Fetch URL: " << fetch_url << std::endl;
        }

        auto push_url = remote.pushurl();
        if (!push_url.empty())
        {
            std::cout << "  Push  URL: " << push_url << std::endl;
        }
        else if (!fetch_url.empty())
        {
            std::cout << "  Push  URL: " << fetch_url << std::endl;
        }

        auto refspecs = remote.refspecs();
        if (!refspecs.empty())
        {
            std::cout << "  HEAD branch: (not yet implemented)" << std::endl;
            for (const auto& refspec : refspecs)
            {
                std::cout << "  " << refspec << std::endl;
            }
        }
    }
}
