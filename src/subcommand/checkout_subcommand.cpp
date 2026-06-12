#include "../subcommand/checkout_subcommand.hpp"

#include <filesystem>
#include <iostream>
#include <set>

#include <git2/oid.h>

#include "../subcommand/status_subcommand.hpp"
#include "../utils/git_exception.hpp"
#include "../wrapper/repository_wrapper.hpp"
#include "../wrapper/status_wrapper.hpp"

checkout_subcommand::checkout_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("checkout", "Switch branches or restore working tree files");

    sub->add_option("<tree-ish|pathspec>", m_positional_args, "Tree-ish to checkout, and/or one/many pathspec(s)\ne.g. checkout <branch>, checkout <tag>, checkout <file> ..., checkout <branch> <file> ...\nNote: use without '--'");
    sub->add_flag("-b", m_create_flag, "Create a new branch before checking it out");
    sub->add_flag("-B", m_force_create_flag, "Create a new branch or reset it if it exists before checking it out");
    sub->add_flag(
        "-f, --force",
        m_force_checkout_flag,
        "When switching branches, proceed even if the index or the working tree differs from HEAD, and even if there are untracked files in the way"
    );

    sub->callback(
        [this]()
        {
            this->run();
        }
    );
}

namespace
{
    void print_no_switch(status_list_wrapper& sl)
    {
        std::cout << "Your local changes to the following files would be overwritten by checkout:" << std::endl;

        for (const auto* entry : sl.get_entry_list(GIT_STATUS_WT_MODIFIED))
        {
            std::cout << "\t" << entry->index_to_workdir->new_file.path << std::endl;
        }
        for (const auto* entry : sl.get_entry_list(GIT_STATUS_WT_DELETED))
        {
            std::cout << "\t" << entry->index_to_workdir->old_file.path << std::endl;
        }

        std::cout << "Please commit your changes or stash them before you switch branches.\nAborting"
                  << std::endl;
        return;
    }
}

std::vector<const char*> convert_paths_to_cstr(const std::vector<std::string>& pathspecs)
{
    std::vector<const char*> pathspec_strings;
    pathspec_strings.reserve(pathspecs.size());
    for (const auto& f : pathspecs)
    {
        pathspec_strings.push_back(f.c_str());
    }
    return pathspec_strings;
}

void checkout_subcommand::checkout_head_files(
    const repository_wrapper& repo,
    const std::vector<std::string>& pathspecs,
    const git_checkout_options& base_options
)
{
    std::vector<const char*> pathspec_strings = convert_paths_to_cstr(pathspecs);

    git_checkout_options options = base_options;
    options.paths.strings = const_cast<char**>(pathspec_strings.data());
    options.paths.count = pathspec_strings.size();

    throw_if_error(git_checkout_head(repo, &options));
}

void checkout_subcommand::checkout_ref_files(
    const repository_wrapper& repo,
    const std::string_view tree_ish,
    const std::vector<std::string>& pathspecs,
    const git_checkout_options& base_options
)
{
    auto obj = repo.revparse_single(tree_ish);
    if (!obj)
    {
        throw git_exception(
            "error: could not resolve tree-ish '" + std::string(tree_ish) + "'",
            git2cpp_error_code::BAD_ARGUMENT
        );
    }

    std::vector<const char*> pathspec_strings = convert_paths_to_cstr(pathspecs);

    git_checkout_options options = base_options;
    options.paths.strings = const_cast<char**>(pathspec_strings.data());
    options.paths.count = pathspec_strings.size();

    throw_if_error(git_checkout_tree(repo, *obj, &options));
}

void checkout_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);

    if (repo.state() != GIT_REPOSITORY_STATE_NONE)
    {
        throw std::runtime_error("Cannot checkout, repository is in unexpected state");
    }

    git_checkout_options options;
    git_checkout_options_init(&options, GIT_CHECKOUT_OPTIONS_VERSION);

    if (m_force_checkout_flag)
    {
        options.checkout_strategy = GIT_CHECKOUT_FORCE;
    }
    else
    {
        options.checkout_strategy = GIT_CHECKOUT_SAFE;
    }

    if (m_positional_args.empty())
    {
        throw std::runtime_error("error: no branch or file specified");
    }

    const std::string& target_name = m_positional_args[0];  // can be a branch or a tag
    const std::vector<std::string> pathspecs(m_positional_args.begin() + 1, m_positional_args.end());

    if (m_create_flag || m_force_create_flag)
    {
        if (!pathspecs.empty())
        {
            throw git_exception("error: '-b' or '-B' does not accept pathspecs.", git2cpp_error_code::BAD_ARGUMENT);
        }

        auto annotated_commit = create_local_branch(repo, target_name, m_force_create_flag);
        checkout_tree(repo, annotated_commit, target_name, options);
        update_head(repo, annotated_commit, target_name);

        std::cout << "Switched to a new branch '" << target_name << "'" << std::endl;
    }
    else if (!pathspecs.empty())
    {
        // Validate all pathspecs before checkout so we can mimic git-like errors
        auto lambda_validate_paths =
            [](repository_wrapper& repo, const std::vector<std::string> pathspecs, std::string directory)
        {
            for (const auto& p : pathspecs)
            {
                if (!std::filesystem::exists(std::filesystem::path(directory) / p) && !repo.does_track(p))
                {
                    throw git_exception(
                        "error: pathspec '" + p + "' did not match any file(s) known to git",
                        git2cpp_error_code::BAD_ARGUMENT
                    );
                }
            }
        };

        // Try tree-ish + pathspec(s)
        if (auto obj = repo.revparse_single(target_name))
        {
            lambda_validate_paths(repo, pathspecs, directory);

            options.checkout_strategy = GIT_CHECKOUT_FORCE;
            checkout_ref_files(repo, target_name, pathspecs, options);
        }
        // Else treat as files
        else
        {
            lambda_validate_paths(repo, pathspecs, directory);

            std::vector<std::string> files = m_positional_args;
            options.checkout_strategy = GIT_CHECKOUT_FORCE;
            checkout_head_files(repo, files, options);
        }
        return;
    }

    auto optional_commit = repo.resolve_local_ref(target_name);
    if (!optional_commit)
    {
        // TODO: handle remote refs

        // Fall back to checking out a unique file
        const std::vector<std::string> file = {target_name};

        if (!std::filesystem::exists(std::filesystem::path(directory) / target_name))
        {
            // Neither a branch/tag nor a file
            throw git_exception(
                "error: pathspec '" + target_name + "' did not match any file(s) known to git",
                git2cpp_error_code::BAD_ARGUMENT
            );
        }

        options.checkout_strategy = GIT_CHECKOUT_FORCE;
        checkout_head_files(repo, file, options);
        return;
    }

    auto sl = status_list_wrapper::status_list(repo);
    try
    {
        checkout_tree(repo, *optional_commit, target_name, options);
        update_head(repo, *optional_commit, target_name);
    }
    catch (const git_exception& e)
    {
        if (sl.has_notstagged_header())
        {
            print_no_switch(sl);
        }
        throw e;
    }

    if (sl.has_notstagged_header())
    {
        bool is_long = false;
        bool is_coloured = false;
        std::set<std::string> tracked_dir_set{};
        print_notstagged(sl, tracked_dir_set, is_long, is_coloured);
    }
    if (sl.has_tobecommited_header())
    {
        bool is_long = false;
        bool is_coloured = false;
        std::set<std::string> tracked_dir_set{};
        print_tobecommited(sl, tracked_dir_set, is_long, is_coloured);
    }

    std::string_view annotated_ref = optional_commit->reference_name();
    if (!annotated_ref.empty() && repo.find_reference(annotated_ref).is_branch())
    {
        std::cout << "Switched to branch '" << target_name << "'" << std::endl;
        print_tracking_info(repo, sl, true, false);
    }
    else
    {
        std::string sha = optional_commit->commit_oid_tostr().substr(0, 7);
        auto commit = repo.find_commit(optional_commit->oid());
        std::string summary = commit.summary();
        std::cout << "HEAD is now at " << sha << " " << summary << std::endl;
    }
}

annotated_commit_wrapper
checkout_subcommand::create_local_branch(repository_wrapper& repo, const std::string_view target_name, bool force)
{
    auto branch = repo.create_branch(target_name, force);
    return repo.find_annotated_commit(branch);
}

void checkout_subcommand::checkout_tree(
    const repository_wrapper& repo,
    const annotated_commit_wrapper& target_annotated_commit,
    const std::string_view target_name,
    const git_checkout_options& options
)
{
    auto target_commit = repo.find_commit(target_annotated_commit.oid());
    throw_if_error(git_checkout_tree(repo, target_commit, &options));
}

void checkout_subcommand::update_head(
    repository_wrapper& repo,
    const annotated_commit_wrapper& target_annotated_commit,
    const std::string_view target_name
)
{
    // Check if HEAD is already detached or not
    const bool head_was_detached = [&]()
    {
        auto head_ref = repo.head();
        return !head_ref.is_branch();
    }();

    // Save previous HEAD info (if it was detached) before changing it (for output message)
    std::optional<commit_wrapper> previous_head_commit;
    std::string previous_head_message;
    if (head_was_detached)
    {
        previous_head_commit = repo.find_commit("HEAD");
        previous_head_message = "Previous HEAD position was "
                                + std::string(previous_head_commit.value().commit_oid_tostr().substr(0, 7))
                                + " " + previous_head_commit.value().summary();
    }

    std::string_view annotated_ref = target_annotated_commit.reference_name();
    if (!annotated_ref.empty())
    {
        auto ref = repo.find_reference(annotated_ref);
        if (ref.is_branch())
        {
            if (head_was_detached)
            {
                std::cout << previous_head_message << std::endl;
            }
            repo.set_head(annotated_ref);
            return;
        }
    }

    repo.set_head_detached(target_annotated_commit);

    if (head_was_detached)
    {
        // Only print "Previous HEAD position..." if HEAD was already detached before and if there is an
        // actual checkout
        auto new_head_commit = repo.find_commit("HEAD");
        if (!git_oid_equal(&previous_head_commit.value().oid(), &new_head_commit.oid()))
        {
            std::cout << previous_head_message << std::endl;
        }
    }
    else
    {
        // Only print the detached-HEAD advice if HEAD was not already detached.
        std::cout << "Note: switching to '" << target_name << "'." << std::endl;
        std::cout << std::endl;
        std::cout << "You are in 'detached HEAD' state. You can look around, make experimental" << std::endl;
        std::cout << "changes and commit them, and you can discard any commits you make in this" << std::endl;
        std::cout << "state without impacting any branches by switching back to a branch." << std::endl;
        std::cout << std::endl;

        // TODO: add to the following when the switch subcommand is implemented:
        // std::cout << "If you want to create a new branch to retain commits you create, you may" <<
        // std::endl; std::cout << "do so (now or later) by using -c with the switch command. Example:" <<
        // std::endl; std::cout << "  git switch -c <new-branch-name>" << std::endl; std::cout << std::endl;
        // std::cout << "Or undo this operation with:" << std::endl;
        // std::cout << std::endl;
        // std::cout << "  git switch -" << std::endl;
        // std::cout << std::endl;
        // TODO: add the following later
        // std::cout << "Turn off this advice by setting config variable advice.detachedHead to false"
        //             << std::endl;
    }
}
