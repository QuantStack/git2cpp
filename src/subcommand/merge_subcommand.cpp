#include <cassert>
#include <git2/reset.h>
#include <git2/types.h>
#include <iostream>
#include <termcolor/termcolor.hpp>

#include "merge_subcommand.hpp"
#include "../wrapper/status_wrapper.hpp"


merge_subcommand::merge_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("merge", "Join two or more development histories together");

    sub->add_option("<branch>", m_branches_to_merge, "Branch(es) to merge");
    // sub->add_flag("--no-ff", m_no_ff, "Create a merge commit in all cases, even when the merge could instead be resolved as a fast-forward.");
    // sub->add_flag("--commit", m_commit, "Perform the merge and commit the result. This option can be used to override --no-commit.");
    sub->add_flag("--no-commit", m_no_commit, "With --no-commit perform the merge and stop just before creating a merge commit, to give the user a chance to inspect and further tweak the merge result before committing. \nNote that fast-forward updates do not create a merge commit and therefore there is no way to stop those merges with --no-commit. Thus, if you want to ensure your branch is not changed or updated by the merge command, use --no-ff with --no-commit.");
    sub->add_flag("--abort", m_abort, "Abort the current conflict resolution process, and try to reconstruct the pre-merge state. If an autostash entry is present, apply it to the worktree.\nIf there were uncommitted worktree changes present when the merge started, git merge --abort will in some cases be unable to reconstruct these changes. It is therefore recommended to always commit or stash your changes before running git merge.\ngit merge --abort is equivalent to git reset --merge when MERGE_HEAD is present unless MERGE_AUTOSTASH is also present in which case git merge --abort applies the stash entry to the worktree whereas git reset --merge will save the stashed changes in the stash list.");
    sub->add_flag("--quit", m_quit, "Forget about the current merge in progress. Leave the index and the working tree as-is. If MERGE_AUTOSTASH is present, the stash entry will be saved to the stash list.");
    sub->add_flag("--continue", m_continue, "After a git merge stops due to conflicts you can conclude the merge by running git merge --continue");   //  (see "HOW TO RESOLVE CONFLICTS" section below).

    sub->callback([this]() { this->run(); });
}

annotated_commit_list_wrapper merge_subcommand::resolve_heads(const repository_wrapper& repo)
{
    std::vector<annotated_commit_wrapper> commits_to_merge;
    commits_to_merge.reserve(m_branches_to_merge.size());

    for (const auto branch_name:m_branches_to_merge)
    {
        std::optional<annotated_commit_wrapper> commit = repo.resolve_local_ref(branch_name);
        if (commit.has_value())
        {
            commits_to_merge.push_back(std::move(commit).value());
        }
    }
    return annotated_commit_list_wrapper(std::move(commits_to_merge));
}

annotated_commit_list_wrapper resolve_mergeheads(const repository_wrapper& repo, const std::vector<git_oid>& oid_list)
{
    std::vector<annotated_commit_wrapper> commits_to_merge;
    commits_to_merge.reserve(oid_list.size());

    for (const auto& id:oid_list)
    {
        std::optional<annotated_commit_wrapper> commit = repo.find_annotated_commit(id);
        if (commit.has_value())
        {
            commits_to_merge.push_back(std::move(commit).value());
        }
    }
    return annotated_commit_list_wrapper(std::move(commits_to_merge));
}

void perform_fastforward(repository_wrapper& repo, const git_oid& target_oid, int is_unborn)
{
    const git_checkout_options ff_checkout_options = GIT_CHECKOUT_OPTIONS_INIT;

    auto lambda_get_target_ref = [] (auto repo, auto is_unborn)
    {
        if (!is_unborn)
        {
            return repo->head();
        }
        else
        {
            return repo->find_reference("HEAD");
        }
    };
    reference_wrapper target_ref = lambda_get_target_ref(&repo, is_unborn);

    object_wrapper target = repo.find_object(target_oid, GIT_OBJECT_COMMIT);

    repo.checkout_tree(target, ff_checkout_options);

    target_ref.write_new_ref(target_oid);
}

void merge_subcommand::create_merge_commit(
    repository_wrapper& repo,
    const index_wrapper& index,
    const std::vector<std::string>& branches_to_merge,
    const annotated_commit_list_wrapper& commits_to_merge,
    size_t num_commits_to_merge)
{
    auto head_ref = repo.head();
    auto merge_ref = repo.find_reference_dwim(branches_to_merge.front());
    auto merge_commit = repo.resolve_local_ref(branches_to_merge.front()).value();

    std::vector<commit_wrapper> parents_list;
    parents_list.reserve(num_commits_to_merge + 1);
    parents_list.push_back(std::move(head_ref.peel<commit_wrapper>()));
    for (size_t i=0; i<num_commits_to_merge; ++i)
    {
        parents_list.push_back(repo.find_commit(commits_to_merge[i].oid()));
    }
    auto parents = commit_list_wrapper(std::move(parents_list));

    auto author_committer_sign = signature_wrapper::get_default_signature_from_env(repo);
    std::string author_name;
    author_name = author_committer_sign.first.name();
    std::string author_email;
    author_email = author_committer_sign.first.email();
    auto author_committer_sign_now = signature_wrapper::signature_now(author_name, author_email, author_name, author_email);

    // TODO: add a prompt to edit the merge message
    std::string msg_target = merge_ref ? merge_ref->short_name() : git_oid_tostr_s(&(merge_commit.oid()));
    msg_target = "\'" + msg_target + "\'";
	std::string msg = merge_ref ? "Merge branch " : "Merge commit ";
	msg.append(msg_target);

	repo.create_commit(author_committer_sign_now, msg, std::optional<commit_list_wrapper>(std::move(parents)));

	repo.state_cleanup();
}

// This function is used as a callback in git_repository_mergehead_foreach and therefore its type must be git_repository_mergehead_foreach_cb.
int populate_list(const git_oid* oid, void* payload)
{
    auto* l = reinterpret_cast<std::vector<git_oid>*>(payload);
    l->push_back(*oid);
    return 0;
}

void merge_subcommand::run()
{
    auto directory = get_current_git_path();
    auto bare = false;
    auto repo = repository_wrapper::open(directory);
    auto state = repo.state();
    index_wrapper index = repo.make_index();
    stream_colour_fn yellow = termcolor::yellow;

    if (state == GIT_REPOSITORY_STATE_MERGE)
    {
        if (m_abort)
        {
            // git merge --abort is equivalent to git reset --merge when MERGE_HEAD is present
            // unless MERGE_AUTOSTASH is also present in which case git merge --abort applies
            // the stash entry to the worktree whereas git reset --merge will save the stashed
            // changes in the stash list.

            if (m_quit | m_continue)
            {
                std::cout << "fatal: --abort expects no arguments" << std::endl;   // TODO: add the help info
                return;
            }

            std::cout << "Warning: 'merge --abort' is not implemented yet. A 'reset --hard HEAD' will be executed." << std::endl;
            std::cout << "Do you want to continue [y/N] ?" << std::endl;
            std::string answer;
            std::cin >> answer;
            if (answer == "y")
            {
                repo.state_cleanup();
                index.conflict_cleanup();

                git_checkout_options options;
                git_checkout_options_init(&options, GIT_CHECKOUT_OPTIONS_VERSION);
                auto head_ref = repo.head();
                repo.reset(head_ref.peel<object_wrapper>(), GIT_RESET_HARD, options);
            }
            else
            {
                std::cout << "Abort." << std::endl;   // maybe another message would be more clear?
            }
            return;
        }
        else if (m_quit)
        {
            // Forget about the current merge in progress. Leave the index and the working tree as-is.
            // If MERGE_AUTOSTASH is present, the stash entry will be saved to the stash list.
            //

            // if (m_continue)
            // {
            //     std::cout << "fatal: --abort expects no arguments" << std::endl;   // TODO: add the help info
            //     return;
            // }

            // problem: can't do a reset if the state is not cleaned up, but it shouldn't be.
            // Idem for the index and the conflicts.

            // repo.state_cleanup();
            // index.conflict_cleanup();

            // git_checkout_options options;
            // git_checkout_options_init(&options, GIT_CHECKOUT_OPTIONS_VERSION);
            // auto head_ref = repo.head();
            // repo.reset(head_ref.peel<object_wrapper>(), GIT_RESET_SOFT, options);

            std::cout << "merge --quit is not implemented yet." << std::endl;
            return;
        }
        else if (m_continue)
		{
		    auto sl = status_list_wrapper::status_list(repo);
		    if (!sl.has_unmerged_header())
			{
			    // std::string commit_message = "Merge branch ";    // how to get the name of the branch the merge was started on ?
 			    // auto author_committer_signatures = signature_wrapper::get_default_signature_from_env(repo);
                // repo.create_commit(author_committer_signatures, commit_message, std::nullopt);

                std::vector<git_oid> oid_list;
                git_repository_mergehead_foreach(repo, populate_list, &oid_list);

                annotated_commit_list_wrapper commits_to_merge = resolve_mergeheads(repo, oid_list);
                size_t num_commits_to_merge = commits_to_merge.size();

                std::vector<std::string> branches_to_merge_names;
                for (const auto& id:oid_list)
				{
				    git_reference_iterator* iter;
					git_reference_iterator_new(&iter, repo);
					git_reference* ref;
					git_reference_next(&ref, iter);
					if (git_oid_equal(git_reference_target(ref), &id))
					{
	                    auto name = git_reference_name(ref);
						branches_to_merge_names.push_back(name);
					}
					git_reference_free(ref);
				}

                create_merge_commit(repo, index, branches_to_merge_names, commits_to_merge, num_commits_to_merge);
                std::cout << "Merge made" << std::endl;     // TODO: change the outpout to something like this: 3c22161 (HEAD -> master) Merge branch 'foregone'

                repo.state_cleanup();
                index.conflict_cleanup();
			    return;
			}
			else
			{
			    auto entry_status = get_status_msg(GIT_STATUS_CONFLICTED).short_mod;
                const auto& entry_list = sl.get_entry_list(GIT_STATUS_CONFLICTED);
                for (auto* entry : entry_list)
			    {
					git_diff_delta* diff_delta = entry->head_to_index; //ou entry->index_to_workdir ???
                    const char* old_path = diff_delta->old_file.path;
                    std::cout << entry_status << "\t" << old_path << std::endl;
				}
			    std::cout << "error: Committing is not possible because you have unmerged files." << std::endl;
			}
		}
        else
        {
            std::cout << "error: Merging is not possible because you have unmerged files." << std::endl;
        }
        std::cout << yellow << "hint: Fix them up in the work tree, and then use 'git add/rm <file>'" << std::endl;
        std::cout << "hint: as appropriate to mark resolution and make a commit." << termcolor::reset << std::endl;
        std::cout << "fatal: Exiting because of an unresolved conflict." << std::endl;
        return;
    }
    else
    {
        if (m_abort)
        {
            std::cout << "fatal: There is no merge to abort (MERGE_HEAD missing)." << std::endl;
            return;
        }
        if (m_continue)
        {
            std::cout << "fatal: There is no merge in progress (MERGE_HEAD missing)." << std::endl;
            return;
        }
    }

    if (state != GIT_REPOSITORY_STATE_NONE)  // Could this be a "else if before the "else" above ?
     {
        std::cout << "repository is in unexpected state " << state <<std::endl;
    }

    git_merge_analysis_t analysis;
	git_merge_preference_t preference;
	annotated_commit_list_wrapper commits_to_merge = resolve_heads(repo);
	size_t num_commits_to_merge = commits_to_merge.size();
	git_annotated_commit** c_commits_to_merge = commits_to_merge;
	auto commits_to_merge_const = const_cast<const git_annotated_commit**>(c_commits_to_merge);

    throw_if_error(git_merge_analysis(&analysis, &preference, repo, commits_to_merge_const, num_commits_to_merge));

    if (analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE)
    {
        std::cout << "Already up-to-date" << std::endl;
        return;
    }
    else if (analysis & GIT_MERGE_ANALYSIS_UNBORN ||
             (analysis & GIT_MERGE_ANALYSIS_FASTFORWARD &&
             !(preference & GIT_MERGE_PREFERENCE_NO_FASTFORWARD)))
    {
        if (analysis & GIT_MERGE_ANALYSIS_UNBORN)
        {
            std::cout << "Unborn" << std::endl;
        }
        else
        {
            std::cout << "Fast-forward" << std::endl;
        }
        const annotated_commit_wrapper& commit = commits_to_merge.front();
        const git_oid target_oid = commit.oid();
        // Since this is a fast-forward, there can be only one merge head.
        assert(num_commits_to_merge == 1);
        perform_fastforward(repo, target_oid, (analysis & GIT_MERGE_ANALYSIS_UNBORN));
    }
    else if (analysis & GIT_MERGE_ANALYSIS_NORMAL)
    {
        git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
		git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

		merge_opts.flags = 0;
		merge_opts.file_flags = GIT_MERGE_FILE_STYLE_DIFF3;

		checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE|GIT_CHECKOUT_ALLOW_CONFLICTS;

		if (preference & GIT_MERGE_PREFERENCE_FASTFORWARD_ONLY)
		{
			std::cout << "Fast-forward is preferred, but only a merge is possible\n" << std::endl;
		}

		throw_if_error(git_merge(repo,
		                         (const git_annotated_commit**)c_commits_to_merge,
								 num_commits_to_merge,
	                             &merge_opts,
								 &checkout_opts));
    }

    if (index.has_conflict())
    {
		index.output_conflicts();
	}
	else if (!m_no_commit)
	{
	    create_merge_commit(repo, index, m_branches_to_merge, commits_to_merge, num_commits_to_merge);
		std::cout << "Merge made" << std::endl;
	}
}
