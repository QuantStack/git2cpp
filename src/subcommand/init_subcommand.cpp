#include <filesystem>
#include "init_subcommand.hpp"

InitSubcommand::InitSubcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("init", "Explanation of init here");

    sub->add_flag("--bare", bare, "info about bare arg");

    // If directory not specified, uses cwd.
    sub->add_option("directory", directory, "info about directory arg")
        ->check(CLI::ExistingDirectory | CLI::NonexistentPath)
        ->default_val(std::filesystem::current_path());

    sub->callback([this]() { this->run(); });
}

void InitSubcommand::run()
{
    RepositoryWrapper::init(directory, bare);
}
