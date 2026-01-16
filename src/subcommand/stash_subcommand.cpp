#include <git2/deprecated.h>
#include <git2/oid.h>
#include <git2/stash.h>
#include <iostream>

#include <git2/remote.h>

#include "../subcommand/stash_subcommand.hpp"
#include "../subcommand/status_subcommand.hpp"
#include "../wrapper/repository_wrapper.hpp"

bool has_subcommand(CLI::App* cmd)
{
    std::vector<std::string> subs = { "push", "pop", "list", "apply" };
    return std::any_of(subs.begin(), subs.end(), [cmd](const std::string& s) { return cmd->got_subcommand(s); });
}

stash_subcommand::stash_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* stash = app.add_subcommand("stash", "Stash the changes in a dirty working directory away");
    auto* push = stash->add_subcommand("push", "");
    auto* list = stash->add_subcommand("list", "");
    auto* pop = stash->add_subcommand("pop", "");
    auto* apply = stash->add_subcommand("apply", "");

    push->add_option("-m,--message", m_message, "");
    pop->add_option("--index", m_index, "");
    apply->add_option("--index", m_index, "");

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

void stash_subcommand::run_pop()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    std::string stash_spec = "stash@{" + std::to_string(m_index) + "}";
    auto stash_obj = repo.revparse_single(stash_spec);
    git_oid stash_id = stash_obj->oid();
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
