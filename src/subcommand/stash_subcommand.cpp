#include <git2/deprecated.h>
#include <git2/oid.h>
#include <git2/stash.h>
#include <iostream>

#include <git2/remote.h>

#include "../subcommand/diff_subcommand.hpp"
#include "../subcommand/stash_subcommand.hpp"
#include "../subcommand/status_subcommand.hpp"

bool has_subcommand(CLI::App* cmd)
{
    std::vector<std::string> subs = { "push", "pop", "list", "apply", "show" };
    return std::any_of(subs.begin(), subs.end(), [cmd](const std::string& s) { return cmd->got_subcommand(s); });
}

stash_subcommand::stash_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* stash = app.add_subcommand("stash", "Stash the changes in a dirty working directory away");
    auto* push = stash->add_subcommand("push", "");
    auto* list = stash->add_subcommand("list", "");
    auto* pop = stash->add_subcommand("pop", "");
    auto* apply = stash->add_subcommand("apply", "");
    auto* show = stash->add_subcommand("show", "Show the changes recorded in the stash as a diff");

    push->add_option("-m,--message", m_message, "");
    pop->add_option("--index", m_index, "");
    apply->add_option("--index", m_index, "");
    show->add_flag("--stat", m_stat_flag, "Generate a diffstat");
    show->add_flag("--shortstat", m_shortstat_flag, "Output only the last line of --stat");
    show->add_flag("--numstat", m_numstat_flag, "Machine-friendly --stat");
    show->add_flag("--summary", m_summary_flag, "Output a condensed summary");

    stash->callback([this,stash]()
       {
           if (!has_subcommand(stash))
           {
               this->run_push();
           }
       });
    push->callback([this]() { this->run_push(); });
    list->callback([this]() { this->run_list(); });
    pop->callback([this]() { this->run_pop(); });
    apply->callback([this]() { this->run_apply(); });
    show->callback([this]() { this->run_show(); });
}

void stash_subcommand::run_push()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);
    auto author_committer_signatures = signature_wrapper::get_default_signature_from_env(repo);

	git_oid stash_id;
    throw_if_error(git_stash_save(&stash_id, repo, author_committer_signatures.first, m_message.c_str(), GIT_STASH_DEFAULT));
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
