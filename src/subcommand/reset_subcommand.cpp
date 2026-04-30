#include "reset_subcommand.hpp"
// #include "../wrapper/index_wrapper.hpp"
#include <stdexcept>

#include "../utils/ansi_code.hpp"
#include "../wrapper/repository_wrapper.hpp"

enum class reset_type
{
    GIT_RESET_SOFT = 1,
    GIT_RESET_MIXED = 2,
    GIT_RESET_HARD = 3
};

reset_subcommand::reset_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("reset", "Reset current HEAD to the specified state");

    sub->add_option("<commit>", m_commit, "The ID of the commit that will become HEAD");

    sub->add_flag(
        "--soft",
        m_soft_flag,
        "Leave your working tree files and the index unchanged. For example, if you have no staged changes, you can use "
            + ansi_code::bold + "git reset --soft HEAD~5" + ansi_code::reset + "; " + ansi_code::bold
            + "git commit" + ansi_code::reset
            + " to combine the last 5 commits into 1 commit. This works even with changes in the working tree, which are left untouched, but such usage can lead to confusion."
    );
    sub->add_flag(
        "--mixed",
        m_mixed_flag,
        "Leave your working directory unchanged. Update the index to match the new HEAD, so nothing will be staged."
    );
    sub->add_flag(
        "--hard",
        m_hard_flag,
        "Overwrite all files and directories with the version from " + ansi_code::bold + "<commit>"
            + ansi_code::reset + ", and may overwrite untracked files. Tracked files not in "
            + ansi_code::bold + "<commit>" + ansi_code::reset
            + " are removed so that the working tree matches " + ansi_code::bold + "<commit>"
            + ansi_code::reset + ". Update the index to match the new HEAD, so nothing will be staged."
    );

    sub->callback(
        [this]()
        {
            this->run();
        }
    );
};

void reset_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

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
