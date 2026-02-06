#include <filesystem>
#include "rm_subcommand.hpp"
#include "../utils/common.hpp"
#include "../utils/git_exception.hpp"
#include "../wrapper/index_wrapper.hpp"
#include "../wrapper/repository_wrapper.hpp"

namespace fs = std::filesystem;

rm_subcommand::rm_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* rm = app.add_subcommand("rm", "Remove files from the working tree and from the index");
    rm->add_option("<pathspec>", m_pathspec, "Files to remove");
    rm->add_flag("-r", m_recursive, "Allow recursive removal when a leading directory name is given");

    rm->add_callback([this]() { this->run(); });
}

void rm_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    index_wrapper index = repo.make_index();

    std::vector<std::string> files;
    std::vector<std::string> directories;

    std::for_each(m_pathspec.begin(), m_pathspec.end(), [&](const std::string& path)
    {
        if (fs::is_directory)
        {
            directories.push_back(path);
        }
        else
        {
            files.push_back(path);
        }
    });

    if (!directories.empty() && !m_recursive)
    {
        std::string msg = "fatal: not removing '" + directories.front() + "' recursively without -r";
        throw git_exception(msg, 128);
    }

    index.remove_entries(files);
    index.remove_directories(directories);
}
