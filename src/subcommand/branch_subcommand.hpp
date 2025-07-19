#pragma once

#include <string>

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"
#include "../wrapper/repository_wrapper.hpp"

class branch_subcommand
{
public:

    explicit branch_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:

    void run_deletion(repository_wrapper& repo);
    void run_creation(repository_wrapper& repo);

    std::string m_branch_name = {};
    bool m_deletion_flag = false;
    bool m_all_flag = false;
    bool m_remote_flag = false;
    bool m_list_flag = false;
    bool m_force_flag = false;
};
