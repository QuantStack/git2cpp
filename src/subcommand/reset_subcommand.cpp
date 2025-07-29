#include "reset_subcommand.hpp"
// #include "../wrapper/index_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"
#include <stdexcept>

enum class reset_type
{
    GIT_RESET_SOFT = 1,
    GIT_RESET_MIXED = 2,
    GIT_RESET_HARD = 3
};

reset_subcommand::reset_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("reset", "Reset current HEAD to the specified state");

    sub->add_option("<commit>", m_commit, "The ID of the commit that will become HEAD");

    sub->add_flag("--soft", m_soft_flag, "");
    sub->add_flag("--mixed", m_mixed_flag, "");
    sub->add_flag("--hard", m_hard_flag, "");

    sub->callback([this]() { this->run(); });
};


void reset_subcommand::run()
{
    auto directory = get_current_git_path();
    auto bare = false;
    auto repo = repository_wrapper::init(directory, bare);

    auto target = repo.revparse_single(m_commit);
    if (!target)
    {
        throw std::runtime_error("Target revision not found.");
    }

    git_checkout_options options;
    git_checkout_options_init(&options, GIT_CHECKOUT_OPTIONS_VERSION);

    git_reset_t reset_type;
    if (m_soft_flag)
    {
        reset_type = GIT_RESET_SOFT;
    }
    if (m_mixed_flag)
    {
        reset_type = GIT_RESET_MIXED;
    }
    if (m_hard_flag)
    {
        reset_type = GIT_RESET_HARD;
        if (m_commit.empty())
        {
            m_commit = "HEAD";
        }
    }

    repo.reset(target.value(), reset_type, options);
}
