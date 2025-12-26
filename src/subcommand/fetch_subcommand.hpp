#pragma once

#include <cstddef>
#include <string>

#include <CLI/CLI.hpp>

#include "../utils/common.hpp"

class fetch_subcommand
{
public:

    explicit fetch_subcommand(const libgit2_object&, CLI::App& app);
    void run();

    std::string m_remote_name;
    size_t m_depth = 0;
    // size_t m_deepen = 0;
    // std::string m_shallow_since;
    // std::string m_shallow_exclude;
    bool m_unshallow = false;
    // bool m_update_shallow = false;
};
