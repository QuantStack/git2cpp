#include <iostream>

#include "../subcommand/clone_subcommand.hpp"
#include "../wrapper/repository_wrapper.hpp"

clone_subcommand::clone_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("clone", "Clone a directory into a new repository");

    sub->add_option("<repository>", m_repository, "The (possibly remote) repository to clone from.")->required();
    sub->add_option("<directory>", m_directory, "The name of a new directory to clone into.");

    sub->callback([this]() { this->run(); });
}

void clone_subcommand::run()
{
    git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
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
    repository_wrapper::clone(m_repository, m_directory, opts);
}
