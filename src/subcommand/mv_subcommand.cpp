#include <filesystem>
#include <system_error>
#include "mv_subcommand.hpp"

#include "../utils/git_exception.hpp"
#include "../wrapper/index_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"

namespace fs = std::filesystem;

mv_subcommand::mv_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("mv" , "Move or rename a file, a directory, or a symlink");
    sub->add_option("<source>", m_source_path, "The path of the source to move")->required()->check(CLI::ExistingFile);
    sub->add_option("<destination>", m_destination_path, "The path of the destination")->required();
    sub->add_flag("-f,--force", m_force, "Force renaming or moving of a file even if the <destination> exists.");

    sub->callback([this]() { this->run(); });
}

void mv_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    bool exists = fs::exists(m_destination_path) && !fs::is_directory(m_destination_path);
    if (exists && !m_force)
    {
        // TODO: replace magic number with enum when diff command is merged
        throw git_exception("destination already exists", 128);
    }

    std::error_code ec;
    fs::rename(m_source_path, m_destination_path, ec);

    if(ec)
    {
        throw git_exception("Could not move file", ec.value());
    }

    auto index = repo.make_index();
    index.remove_entry(m_source_path);
    index.add_entry(m_destination_path);
    index.write();
}
