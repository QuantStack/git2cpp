#pragma once

#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"
#include "../wrapper/repository_wrapper.hpp"

class push_subcommand
{
public:

    explicit push_subcommand(const libgit2_object&, CLI::App& app);
    void run();

    void fill_refspec(repository_wrapper& repo);

private:

    std::string m_remote_name;
    std::vector<std::string> m_refspecs;
    bool m_branches_flag = false;
};
