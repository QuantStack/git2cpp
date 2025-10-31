#include <cassert>
#include <git2/types.h>

#include "merge_subcommand.hpp"
#include <iostream>


merge_subcommand::merge_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("merge", "Join two or more development histories together");

    sub->add_option("<branch>", m_branches_to_merge, "Branch(es) to merge");
    // sub->add_flag("--no-ff", m_no_ff, "");
    // sub->add_flag("--commit", m_commit, "Perform the merge and commit the result. This option can be used to override --no-commit.");
    sub->add_flag("--no-commit", m_no_commit, "With --no-commit perform the merge and stop just before creating a merge commit, to give the user a chance to inspect and further tweak the merge result before committing. \nNote that fast-forward updates do not create a merge commit and therefore there is no way to stop those merges with --no-commit. Thus, if you want to ensure your branch is not changed or updated by the merge command, use --no-ff with --no-commit.");

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

void perform_fastforward(repository_wrapper& repo, const git_oid target_oid, int is_unborn)
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
    const annotated_commit_list_wrapper& commits_to_merge,
    size_t num_commits_to_merge)
{
    auto head_ref = repo.head();
    auto merge_ref = repo.find_reference_dwim(m_branches_to_merge.front());
    auto merge_commit = repo.resolve_local_ref(m_branches_to_merge.front()).value();

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
	std::string msg = merge_ref ? "Merge branch " : "Merge commit ";
	msg.append(msg_target);

	repo.create_commit(author_committer_sign_now, msg, std::optional<commit_list_wrapper>(std::move(parents)));

	repo.state_cleanup();
}

void merge_subcommand::run()
{
    auto directory = get_current_git_path();
    auto bare = false;
    auto repo = repository_wrapper::open(directory);

    auto state = repo.state();
    if (state != GIT_REPOSITORY_STATE_NONE)
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

    index_wrapper index = repo.make_index();

    if (git_index_has_conflicts(index))
    {
        std::cout << "Conflict. To be implemented" << std::endl;
        /* Handle conflicts */
		// output_conflicts(index);
	}
	else if (!m_no_commit)
	{
		create_merge_commit(repo, index, commits_to_merge, num_commits_to_merge);
		printf("Merge made\n");
	}
}
