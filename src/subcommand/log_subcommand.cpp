#include <format>
#include <git2.h>
#include <git2/oid.h>
#include <git2/refs.h>
#include <git2/types.h>
#include <sstream>
#include <string_view>
#include <vector>

#include <termcolor/termcolor.hpp>

#include "log_subcommand.hpp"
#include "../utils/terminal_pager.hpp"
#include "../wrapper/repository_wrapper.hpp"
#include "../wrapper/commit_wrapper.hpp"

log_subcommand::log_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("log", "Shows commit logs");

    sub->add_flag("--format", m_format_flag, "Pretty-print the contents of the commit logs in a given format, where <format> can be one of full and fuller");
    sub->add_option("-n,--max-count", m_max_count_flag, "Limit the output to <number> commits.");
    // sub->add_flag("--oneline", m_oneline_flag, "This is a shorthand for --pretty=oneline --abbrev-commit used together.");

    sub->callback([this]() { this->run(); });
};

void print_time(git_time intime, std::string prefix)
{
	char sign, out[32];
	struct tm *intm;
	int offset, hours, minutes;
	time_t t;

	offset = intime.offset;
	if (offset < 0) {
		sign = '-';
		offset = -offset;
	}
	else
	{
		sign = '+';
	}

	hours   = offset / 60;
	minutes = offset % 60;

	t = (time_t)intime.time + (intime.offset * 60);

	intm = gmtime(&t);
	strftime(out, sizeof(out), "%a %b %e %T %Y", intm);

	std::cout << prefix << out << " " << sign << std::format("{:02d}", hours) << std::format("{:02d}", minutes) <<std::endl;
}

std::vector<std::string> get_tags_for_commit(repository_wrapper& repo, const git_oid& commit_oid)
{
    std::vector<std::string> tags;
    git_strarray tag_names = {0};

    if (git_tag_list(&tag_names, repo) != 0)
    {
        return tags;
    }

    for (size_t i = 0; i < tag_names.count; i++)
    {
        std::string tag_name = tag_names.strings[i];
        std::string ref_name = "refs/tags/" + std::string(tag_name);

        reference_wrapper tag_ref = repo.find_reference(ref_name);
        object_wrapper peeled = tag_ref.peel<object_wrapper>();

        if (git_oid_equal(&peeled.oid(), &commit_oid))
        {
            tags.push_back(std::string(tag_name));
        }
    }

    git_strarray_dispose(&tag_names);   // TODO: refactor git_strarray_wrapper to use it here
    return tags;
}

std::vector<std::string> get_branches_for_commit(repository_wrapper& repo, git_branch_t type, const git_oid& commit_oid, const std::string exclude_branch)
{
    std::vector<std::string> branches;

    auto branch_iter = repo.iterate_branches(type);
    while (auto branch = branch_iter.next())
    {
        const git_oid* branch_target = nullptr;
        git_reference* ref = branch.value();

        if (git_reference_type(ref) == GIT_REFERENCE_DIRECT)
        {
            branch_target = git_reference_target(ref);
        }
        else if (git_reference_type(ref) == GIT_REFERENCE_SYMBOLIC)
        {
            git_reference* resolved = nullptr;
            if (git_reference_resolve(&resolved, ref) == 0)
            {
                branch_target = git_reference_target(resolved);
                git_reference_free(resolved);
            }
        }

        if (branch_target && git_oid_equal(branch_target, &commit_oid))
        {
            std::string branch_name(branch->name());
            if (type == GIT_BRANCH_LOCAL)
            {
                if (branch_name != exclude_branch)
                {
                    branches.push_back(branch_name);
                }
            }
            else
            {
                branches.push_back(branch_name);
            }
        }
    }

    return branches;
}

struct commit_refs
{
    std::string head_branch;
    std::vector<std::string> tags;
    std::vector<std::string> local_branches;
    std::vector<std::string> remote_branches;

    bool has_refs() const {
        return !head_branch.empty() || !tags.empty() ||
               !local_branches.empty() || !remote_branches.empty();
    }
};

commit_refs get_refs_for_commit(repository_wrapper& repo, const git_oid& commit_oid)
{
    commit_refs refs;

    if (!repo.is_head_unborn())
    {
        auto head = repo.head();
        auto head_taget = head.target();
        if (git_oid_equal(head_taget, &commit_oid))
        {
            refs.head_branch = head.short_name();
        }
    }

    refs.tags = get_tags_for_commit(repo, commit_oid);
    refs.local_branches = get_branches_for_commit(repo, GIT_BRANCH_LOCAL, commit_oid, refs.head_branch);
    refs.remote_branches = get_branches_for_commit(repo, GIT_BRANCH_REMOTE, commit_oid, "");

    return refs;
}

void print_refs(const commit_refs& refs)
{
    if (!refs.has_refs())
    {
        return;
    }

    std::cout << " (";

    bool first = true;

    if (!refs.head_branch.empty())
    {
        std::cout << termcolor::bold << termcolor::cyan << "HEAD" << termcolor::reset
                  << termcolor::yellow << " -> " << termcolor::reset
                  << termcolor::bold << termcolor::green << refs.head_branch << termcolor::reset
                  << termcolor::yellow;
        first = false;
    }

    for (const auto& tag :refs.tags)
    {
        if (!first)
        {
            std::cout << ", ";
        }
        std::cout << termcolor::bold << "tag: " << tag << termcolor::reset << termcolor::yellow;
        first = false;
    }

    for (const auto& remote : refs.remote_branches)
    {
        if (!first)
        {
            std::cout << ", ";
        }
        std::cout << termcolor::bold << termcolor::red << remote << termcolor::reset << termcolor::yellow;
        first = false;
    }

    for (const auto& local : refs.local_branches)
    {
        if (!first)
        {
            std::cout << ", ";
        }
        std::cout << termcolor::bold << termcolor::green << local << termcolor::reset << termcolor::yellow;
        first = false;
    }

    std::cout << ")" << termcolor::reset;
}

void print_commit(repository_wrapper& repo, const commit_wrapper& commit, std::string m_format_flag)
{
    std::string buf = commit.commit_oid_tostr();

    signature_wrapper author = signature_wrapper::get_commit_author(commit);
    signature_wrapper committer = signature_wrapper::get_commit_committer(commit);

    stream_colour_fn colour = termcolor::yellow;
    std::cout << colour << "commit " << buf;

    commit_refs refs = get_refs_for_commit(repo, commit.oid());
    print_refs(refs);

    std::cout << termcolor::reset << std::endl;

    if (m_format_flag=="fuller")
    {
        std::cout << "Author:\t    " <<  author.name() << " " << author.email() << std::endl;
        print_time(author.when(), "AuthorDate: ");
        std::cout << "Commit:\t    " <<  committer.name() << " " << committer.email() << std::endl;
        print_time(committer.when(), "CommitDate: ");
    }
    else
    {
        std::cout << "Author:\t" <<  author.name() << " " << author.email() << std::endl;
        if (m_format_flag=="full")
        {
            std::cout << "Commit:\t" <<  committer.name() << " " << committer.email() << std::endl;
        }
        else
        {
            print_time(author.when(), "Date:\t");
        }
    }

    std::string message = commit.message();
    while (!message.empty() && message.back() == '\n')
    {
        message.pop_back();
    }
    std::istringstream message_stream(message);
    std::string line;
    while (std::getline(message_stream, line))
    {
        std::cout << "\n    " << line;
    }
    std::cout << std::endl;
}

void log_subcommand::run()
{
    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);
    // auto branch_name = repo.head().short_name();

    if (repo.is_head_unborn())
    {
        std::cout << "fatal: your current branch 'main' does not have any commits yet" << std::endl;
        return;
    }

    revwalk_wrapper walker = repo.new_walker();
    walker.push_head();

    terminal_pager pager;

    std::size_t i=0;
    git_oid commit_oid;
    while (!walker.next(commit_oid) && i<m_max_count_flag)
    {
        if (i != 0)
        {
            std::cout << std::endl;
        }
        commit_wrapper commit = repo.find_commit(commit_oid);
        print_commit(repo, commit, m_format_flag);
        ++i;
    }

    pager.show();
}
