#pragma once

#include <CLI/CLI.hpp>
#include <limits>

#include "../utils/common.hpp"

class clone_subcommand
{
public:

    explicit clone_subcommand(const libgit2_object&, CLI::App& app);
    void run();

private:

    std::string m_repository = {};
    std::string m_directory = {};
    bool m_bare = false;
    size_t m_depth = std::numeric_limits<size_t>::max();
    // std::string m_shallow_since;
    // std::vector<std::string> m_shallow_exclude;
};
