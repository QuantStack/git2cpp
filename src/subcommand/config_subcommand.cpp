#include <git2/config.h>
#include <git2/types.h>
#include <iostream>

#include <git2/remote.h>

#include "../utils/git_exception.hpp"
#include "../subcommand/config_subcommand.hpp"
#include "../wrapper/config_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"

config_subcommand::config_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* config = app.add_subcommand("config", "Get and set repository or global options");
    auto* list = config->add_subcommand("list", "List all variables set in config file, along with their values.");
    auto* get = config->add_subcommand("get", "Emits the value of the specified key. If key is present multiple times in the configuration, emits the last value. If --all is specified, emits all values associated with key. Returns error code 1 if key is not present.");
    auto* set = config->add_subcommand("set", "Set value for one or more config options. By default, this command refuses to write multi-valued config options. Passing --all will replace all multi-valued config options with the new value, whereas --value= will replace all config options whose values match the given pattern.");
    auto* unset = config->add_subcommand("unset", "Unset value for one or more config options. By default, this command refuses to unset multi-valued keys. Passing --all will unset all multi-valued config options, whereas --value will unset all config options whose values match the given pattern.");

    get->add_option("<name>", m_name, "");
    set->add_option("<name>", m_name, "");
    set->add_option("<value>", m_value, "");
    unset->add_option("<name>", m_name, "");

    // TODO:
    // sub->add_flag("--local", m_local_flag, "");
    // sub->add_flag("--global", m_global_flag, "");
    // sub->add_flag("--system", m_system_flag, "");
    // sub->add_flag("--worktree", m_worktree_flag, "");

    list->callback([this]() { this->run_list(); });
    get->callback([this]() { this->run_get(); });
    set->callback([this]() { this->run_set(); });
    unset->callback([this]() { this->run_unset(); });
}

void config_subcommand::run_list()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);
    auto cfg = repo.get_config();

    git_config_iterator* iter;
    throw_if_error(git_config_iterator_new(&iter, cfg));

    git_config_entry* entry;
    while (git_config_next(&entry, iter) == GIT_OK)
    {
        std::cout << entry->name << "=" << entry->value << std::endl;
    }

    git_config_iterator_free(iter);
}

void config_subcommand::run_get()
{
    if (m_name.empty())
    {
        throw git_exception("error: wrong number of arguments, should be 1", 129);
    }

    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);
    auto cfg = repo.get_config();

    git_config_entry* entry = cfg.get_entry(m_name);
	std::cout << entry->value << std::endl;

	git_config_entry_free(entry);
}

void config_subcommand::run_set()
{
    if (m_name.empty() | m_value.empty())
    {
        throw git_exception("error: wrong number of arguments, should be 2", 129);
    }

    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);
    auto cfg = repo.get_config();

    cfg.set_entry(m_name, m_value);
}

void config_subcommand::run_unset()
{
    if (m_name.empty())
    {
        throw git_exception("error: wrong number of arguments, should be 1", 129);
    }

    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);
    auto cfg = repo.get_config();

    cfg.delete_entry(m_name);
}
