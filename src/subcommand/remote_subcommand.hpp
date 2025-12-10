#pragma once

#include <string>

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"
#include "../wrapper/repository_wrapper.hpp"

class remote_subcommand
{
public:

    explicit remote_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:

    void run_list(const repository_wrapper& repo);
    void run_add(repository_wrapper& repo);
    void run_remove(repository_wrapper& repo);
    void run_rename(repository_wrapper& repo);
    void run_seturl(repository_wrapper& repo);
    void run_show(const repository_wrapper& repo);

    CLI::App* m_subcommand = nullptr;
    std::string m_operation;
    std::string m_remote_name;
    std::string m_url;
    std::string m_old_name;
    std::string m_new_name;
    bool m_verbose_flag = false;
    bool m_push_flag = false;
};
