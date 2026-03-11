#include <filesystem>
#include "init_subcommand.hpp"
#include "../wrapper/repository_wrapper.hpp"

init_subcommand::init_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("init", "Explanation of init here");

    sub->add_flag("--bare", m_bare, "info about bare arg");

    // If directory not specified, uses cwd.
    sub->add_option("directory", m_directory, "info about directory arg")
        ->check(CLI::ExistingDirectory | CLI::NonexistentPath)
        ->default_val(get_current_git_path());
    sub->add_option("-b,--initial-branch", m_branch, "Use <branch-name> for the initial branch in the newly created repository. If not specified, fall back to the default name.");

    sub->callback([this]() { this->run(); });
}

void init_subcommand::run()
{
    std::filesystem::path target_dir = m_directory;
    bool reinit = std::filesystem::exists(target_dir / ".git" / "HEAD");

    std::string git_path;
    if (m_branch.empty())
    {
        auto repo = repository_wrapper::init(m_directory, m_bare);
        git_path = repo.git_path();
    }
    else
    {
        git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;
        if (m_bare) {opts.flags |= GIT_REPOSITORY_INIT_BARE;}
        opts.initial_head = m_branch.c_str();

        auto repo = repository_wrapper::init_ext(m_directory, &opts);
        git_path = repo.git_path();
    }

    std::string path;
    if (m_bare)
    {
        size_t pos = git_path.find(".git/");
        path = git_path.substr(0, pos);
    }
    else
    {
        path = git_path;
    }

    if (reinit)
    {
        std::cout << "Reinitialized existing Git repository in " << path <<std::endl;
    }
    else
    {
        std::cout << "Initialized empty Git repository in " << path <<std::endl;
    }
}
