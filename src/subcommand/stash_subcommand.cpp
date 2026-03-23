#include "../subcommand/stash_subcommand.hpp"

#include <iostream>

#include <git2/deprecated.h>
#include <git2/oid.h>
#include <git2/remote.h>
#include <git2/stash.h>

#include "../subcommand/diff_subcommand.hpp"
#include "../subcommand/status_subcommand.hpp"
#include "../utils/ansi_code.hpp"

bool has_subcommand(CLI::App* cmd)
{
    std::vector<std::string> subs = {"push", "pop", "list", "apply", "show"};
    return std::any_of(
        subs.begin(),
        subs.end(),
        [cmd](const std::string& s)
        {
            return cmd->got_subcommand(s);
        }
    );
}

stash_subcommand::stash_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* stash = app.add_subcommand("stash", "Stash the changes in a dirty working directory away\n");
    auto* push = stash->add_subcommand("push", "Save your local modifications to a new stash entry and roll them back to " + ansi_code::bold + "HEAD " + ansi_code::reset + "(in the working tree and in the index). The <message> part is optional and gives the description along with the stashed state.\n");
    auto* list = stash->add_subcommand("list", "List the stash entries that you currently have. Each stash entry is listed with its name (e.g. " + ansi_code::bold + "stash@" + ansi_code::reset + "{0} is the latest entry, " + ansi_code::bold + "stash@" + ansi_code::reset + "{1} is the one before, etc.), the name of the branch that was current when the entry was made, and a short description of the commit the entry was based on.\n");  //TODO: check if shows all of that
    auto* pop = stash->add_subcommand("pop", "Remove a single stashed state from the stash list and apply it on top of the current working tree state, i.e., do the inverse operation of " + ansi_code::bold + "git stash push.\n" + ansi_code::reset);
    auto* apply = stash->add_subcommand("apply", "Like " + ansi_code::bold + "pop" + ansi_code::reset + ", but do not remove the state from the stash list. Unlike " + ansi_code::bold + "pop" + ansi_code::reset + ", <stash> may be any commit that looks like a commit created by " + ansi_code::bold + "stash push" + ansi_code::reset + " or " + ".\n");  //TODO: add when "create" is implemented: ansi_code::bold + "stash create" + ansi_code::reset +
    auto* show = stash->add_subcommand("show", "Show the changes recorded in the stash entry as a diff between the stashed contents and the commit back when the stash entry was first created.\n");

    push->add_option("-m,--message", m_message, "");
    pop->add_option("--index", m_index, "This option is only valid for " + ansi_code::bold + "pop" + ansi_code::reset + " and " + ansi_code::bold + "apply" + ansi_code::reset + " commands.\n");
    apply->add_option("--index", m_index, "This option is only valid for " + ansi_code::bold + "pop" + ansi_code::reset + " and " + ansi_code::bold + "apply" + ansi_code::reset + " commands.\n");
    show->add_flag("--stat", m_stat_flag, "Generate a diffstat\n");
    show->add_flag("--shortstat", m_shortstat_flag, "Output only the last line of --stat\n");
    show->add_flag("--numstat", m_numstat_flag, "Machine-friendly --stat\n");
    show->add_flag("--summary", m_summary_flag, "Output a condensed summary\n");

    stash->callback(
        [this, stash]()
        {
            if (!has_subcommand(stash))
            {
                this->run_push();
            }
        }
    );
    push->callback(
        [this]()
        {
            this->run_push();
        }
    );
    list->callback(
        [this]()
        {
            this->run_list();
        }
    );
    pop->callback(
        [this]()
        {
            this->run_pop();
        }
    );
    apply->callback(
        [this]()
        {
            this->run_apply();
        }
    );
    show->callback(
        [this]()
        {
            this->run_show();
        }
    );
}

void stash_subcommand::run_push()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);
    auto author_committer_signatures = signature_wrapper::get_default_signature_from_env(repo);

    git_oid stash_id;
    throw_if_error(
        git_stash_save(&stash_id, repo, author_committer_signatures.first, m_message.c_str(), GIT_STASH_DEFAULT)
    );
    auto stash = repo.find_commit(stash_id);
    std::cout << "Saved working directory and index state " << stash.summary() << std::endl;
}

static int list_stash_cb(size_t index, const char* message, const git_oid* stash_id, void* payload)
{
    std::cout << "stash@{" << index << "}: " << message << std::endl;
    return 0;
}

void stash_subcommand::run_list()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    throw_if_error(git_stash_foreach(repo, list_stash_cb, NULL));
}

git_oid stash_subcommand::resolve_stash_commit(repository_wrapper& repo)
{
    std::string stash_spec = "stash@{" + std::to_string(m_index) + "}";
    auto stash_obj = repo.revparse_single(stash_spec);
    git_oid stash_id = stash_obj->oid();
    return stash_id;
}

void stash_subcommand::run_pop()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    git_oid stash_id = resolve_stash_commit(repo);
    char id_string[GIT_OID_HEXSZ + 1];
    git_oid_tostr(id_string, sizeof(id_string), &stash_id);

    throw_if_error(git_stash_pop(repo, m_index, NULL));
    status_run();
    std::cout << "Dropped refs/stash@{" << m_index << "} (" << id_string << ")" << std::endl;
}

void stash_subcommand::run_apply()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    throw_if_error(git_stash_apply(repo, m_index, NULL));
    status_run();
}

void stash_subcommand::run_show()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    git_oid stash_id = resolve_stash_commit(repo);
    commit_wrapper stash_commit = repo.find_commit(stash_id);

    if (git_commit_parentcount(stash_commit) < 1)
    {
        throw std::runtime_error("stash show: stash commit has no parents");
    }

    commit_wrapper parent_commit = stash_commit.get_parent(0);

    tree_wrapper stash_tree = stash_commit.tree();
    tree_wrapper parent_tree = parent_commit.tree();

    git_diff_options diff_opts = GIT_DIFF_OPTIONS_INIT;

    diff_wrapper diff = repo.diff_tree_to_tree(parent_tree, stash_tree, &diff_opts);

    bool use_colour = true;
    if (!m_shortstat_flag && !m_numstat_flag && !m_summary_flag)
    {
        m_stat_flag = true;
    }
    print_stats(diff, use_colour, m_stat_flag, m_shortstat_flag, m_numstat_flag, m_summary_flag);
}
