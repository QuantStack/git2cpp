#include <cassert>
#include <git2/types.h>

#include "merge_subcommand.hpp"
#include "../wrapper/repository_wrapper.hpp"


merge_subcommand::merge_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("merge", "Join two or more development histories together");

    sub->add_option("<branch>", m_branches_to_merge, "Branch(es) to merge");

    sub->callback([this]() { this->run(); });
}

annotated_commit_list_wrapper resolve_heads(const repository_wrapper& repo, std::vector<std::string> m_branches_to_merge)
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

void perform_fastforward(repository_wrapper& repo, const git_oid* target_oid, int is_unborn)
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
    auto target_ref = lambda_get_target_ref(&repo, is_unborn);

    auto target = repo.find_object(target_oid, GIT_OBJECT_COMMIT);

    repo.checkout_tree(target, &ff_checkout_options);

    auto new_target_ref = target_ref.new_ref(target_oid);
}

// static void create_merge_commit(repository_wrapper repo, index_wrapper index, std::vector<std::string> m_branches_to_merge,
//     std::vector<annotated_commit_wrapper> commits_to_merge)
// {
//     auto head_ref = repo.head();
//     auto merge_ref = repo.find_reference_dwim(m_branches_to_merge.front());
//     // if (ref)
//     // {
//     //     auto merge_ref = std::move(ref).value();
//     // }
//     auto merge_commit = repo.resolve_local_ref(m_branches_to_merge.front()).value();

//     size_t annotated_count = commits_to_merge.size();
//     std::vector<commit_wrapper> parents_list;
//     parents_list.reserve(annotated_count + 1);
//     parents_list.push_back(std::move(head_ref.peel<commit_wrapper>()));
//     for (size_t i=0; i<annotated_count; ++i)
//     {
//         parents_list.push_back(repo.find_commit(commits_to_merge[i].oid()));
//     }
//     auto parents = commit_list_wrapper(std::move(parents_list));

//     auto author_committer_sign = signature_wrapper::get_default_signature_from_env(repo);
//     std::string author_name;
//     author_name = author_committer_sign.first.name();
//     std::string author_email;
//     author_email = author_committer_sign.first.email();
//     auto author_committer_sign_now = signature_wrapper::signature_now(author_name, author_email, author_name, author_email);

//     std::string msg_target = NULL;
//     if (merge_ref)
//     {
//         msg_target = merge_ref->short_name();
//     }
//     else
//     {
//         msg_target = git_oid_tostr_s(&(merge_commit.oid()));
//     }

// 	std::string msg;
// 	msg = "Merge ";
// 	if (merge_ref)
// 	{
// 	    msg.append("branch ");
// 	}
// 	else
// 	{
// 	    msg.append("commit ");
// 	}
// 	msg.append(msg_target);
// 	std::cout << msg << std::endl;

// 	repo.create_commit(author_committer_sign_now, msg, std::optional<commit_list_wrapper>(std::move(parents)));
// }

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
	annotated_commit_list_wrapper commits_to_merge = resolve_heads(repo, m_branches_to_merge);
	size_t num_commits_to_merge = commits_to_merge.size();
	git_annotated_commit** c_commits_to_merge = commits_to_merge;
	auto commits_to_merge_const = const_cast<const git_annotated_commit**>(c_commits_to_merge);

    throw_if_error(git_merge_analysis(&analysis, &preference, repo, commits_to_merge_const, num_commits_to_merge));

    if (analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE)
    {
        std::cout << "Already up-to-date" << std::endl;
    }
    else if (analysis & GIT_MERGE_ANALYSIS_UNBORN ||
             (analysis & GIT_MERGE_ANALYSIS_FASTFORWARD &&
             !(preference & GIT_MERGE_PREFERENCE_NO_FASTFORWARD)))
    {
        const git_oid* target_oid;
        if (analysis & GIT_MERGE_ANALYSIS_UNBORN)
        {
            std::cout << "Unborn" << std::endl;
        }
        else
        {
            std::cout << "Fast-forward" << std::endl;
        }
        const annotated_commit_wrapper& commit = commits_to_merge.front();
        target_oid = &commit.oid();
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
			// how to break ?
		}

		// git_merge(repo,
	 //                (const git_annotated_commit **)opts.annotated, opts.annotated_count,
	 //                &merge_opts, &checkout_opts);
    }

 //    if (git_index_has_conflicts(index)) {
	// 	/* Handle conflicts */
	// 	output_conflicts(index);
	// } else if (!opts.no_commit) {
	// 	create_merge_commit(repo, index, &opts);
	// 	printf("Merge made\n");
	// }
}
