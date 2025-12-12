#include "revparse_subcommand.hpp"
#include "../wrapper/repository_wrapper.hpp"
#include <ios>
#include <stdexcept>

revparse_subcommand::revparse_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("rev-parse", "Pick out and message parameters");

    sub->add_flag("--is-bare-repository", m_is_bare_repository_flag);

    sub->callback([this]() { this->run(); });
}

void revparse_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    if (m_is_bare_repository_flag)
    {
        std::cout << std::boolalpha << repo.is_bare() << std::endl;
    }
    else
    {
        std::cout << "revparse only supports --is-bare-repository for now" << std::endl;
    }
}
