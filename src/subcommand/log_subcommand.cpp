#include <format>
#include <git2.h>
#include <git2/revwalk.h>
#include <git2/types.h>
#include <string_view>

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

void print_commit(const commit_wrapper& commit, std::string m_format_flag)
{
    std::string buf = commit.commit_oid_tostr();

    signature_wrapper author = signature_wrapper::get_commit_author(commit);
    signature_wrapper committer = signature_wrapper::get_commit_committer(commit);

    stream_colour_fn colour = termcolor::yellow;
    std::cout << colour << "commit " << buf << termcolor::reset << std::endl;
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
    std::cout << "\n    " << git_commit_message(commit) << "\n" << std::endl;
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
        commit_wrapper commit = repo.find_commit(commit_oid);
        print_commit(commit, m_format_flag);
        ++i;
    }

    pager.show();
}
