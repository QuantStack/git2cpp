#pragma once

#include <string>

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"
#include "../wrapper/rebase_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"

class rebase_subcommand
{
public: 

    explicit rebase_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:

    void run_rebase(repository_wrapper& repo);
    void run_abort(repository_wrapper& repo);
    void run_continue(repository_wrapper& repo);
    void run_skip(repository_wrapper& repo);
    void run_quit(repository_wrapper& repo);

    annotated_commit_wrapper resolve_ref(const repository_wrapper& repo, const std:: string& ref_name) const;

    void perform_rebase(repository_wrapper& repo, rebase_wrapper& rebase);

    std::string m_upstream = {};
    std::string m_branch = {};
    std::string m_onto = {};

    bool m_abort = false;
    bool m_continue = false;
    bool m_skip = false;
    bool m_quit = false;
};
