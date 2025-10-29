#include <cassert>
#include <git2/types.h>

#include "merge_subcommand.hpp"
// #include "../wrapper/repository_wrapper.hpp"


merge_subcommand::merge_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("merge", "Join two or more development histories together");

    sub->add_option("<branch>", m_branches_to_merge, "Branch(es) to merge");

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
}
