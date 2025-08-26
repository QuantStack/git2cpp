#include <CLI/CLI.hpp>
#include <git2.h>  // For version number only
#include <iostream>

#include "utils/git_exception.hpp"
#include "version.hpp"
#include "subcommand/add_subcommand.hpp"
#include "subcommand/branch_subcommand.hpp"
#include "subcommand/checkout_subcommand.hpp"
#include "subcommand/clone_subcommand.hpp"
#include "subcommand/commit_subcommand.hpp"
#include "subcommand/init_subcommand.hpp"
#include "subcommand/log_subcommand.hpp"
#include "subcommand/reset_subcommand.hpp"
#include "subcommand/status_subcommand.hpp"

int main(int argc, char** argv)
{
    int exit_code = 0;
    try
    {
        const libgit2_object lg2_obj;
        CLI::App app{"Git using C++ wrapper of libgit2"};

        // Top-level command options.
        auto version = app.add_flag("-v,--version", "Show version");

        // Sub commands
        init_subcommand init(lg2_obj, app);
        status_subcommand status(lg2_obj, app);
        add_subcommand add(lg2_obj, app);
        branch_subcommand branch(lg2_obj, app);
        checkout_subcommand checkout(lg2_obj, app);
        clone_subcommand clone(lg2_obj, app);
        commit_subcommand commit(lg2_obj, app);
        reset_subcommand reset(lg2_obj, app);
        log_subcommand log(lg2_obj, app);

        app.require_subcommand(/* min */ 0, /* max */ 1);

        CLI11_PARSE(app, argc, argv);

        if (version->count())
        {
            std::cout << "git2cpp version " << GIT2CPP_VERSION_STRING << " (libgit2 " << LIBGIT2_VERSION << ")" << std::endl;
        }
        else if (app.get_subcommands().size() == 0)
        {
            std::cout << app.help() << std::endl;
        }
    }
    catch (const CLI::Error& e)
    {
        std::cerr << e.what() << std::endl;
        exit_code = 1;
    }
    catch (const git_exception& e)
    {
        std::cerr << e.what() << std::endl;
        exit_code = e.error_code();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit_code = 1;
    }

    return exit_code;
}
