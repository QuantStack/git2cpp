#include "showref_subcommand.hpp"

#include <git2.h>

#include "../wrapper/repository_wrapper.hpp"

showref_subcommand::showref_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("show-ref", "List references in a local repository");

    sub->callback(
        [this]()
        {
            this->run();
        }
    );
};

void showref_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    auto repo_refs = repo.refs_list();

    for (auto r:repo_refs)
    {
        git_oid oid = repo.ref_name_to_id(r);
        std::cout << oid_to_hex(oid) << " " << r << std::endl;
    }
}
