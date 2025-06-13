#pragma once

#include <CLI/CLI.hpp>
#include <string>

#include "src/wrapper/repository_wrapper.hpp"

class InitSubcommand
{
public:

    explicit InitSubcommand(const libgit2_object&, CLI::App& app);
    void run();

private:
    bool bare;
    std::string directory;
};
