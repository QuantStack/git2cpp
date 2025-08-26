// #include <iostream>
// #include <ostream>
// #include <string>

#include <git2.h>
#include <git2/revwalk.h>
#include <git2/types.h>

#include "log_subcommand.hpp"
#include "../wrapper/repository_wrapper.hpp"

// TODO: put in another file
/** Size (in bytes) of a raw/binary sha1 oid */
#define GIT_OID_SHA1_SIZE       20
/** Size (in bytes) of a hex formatted sha1 oid */
#define GIT_OID_SHA1_HEXSIZE   (GIT_OID_SHA1_SIZE * 2)

log_subcommand::log_subcommand(const libgit2_object&, CLI::App& app)
{
    auto *sub = app.add_subcommand("log", "Shows commit logs");

    sub->add_flag("-n,--max-count", m_max_count_flag, "Limit the output to <number> commits.");
    // sub->add_flag("--oneline", m_oneline_flag, "This is a shorthand for --pretty=oneline --abbrev-commit used together.");

    sub->callback([this]() { this->run(); });
};

void print_time(git_time intime, const char *prefix)
{
	char sign, out[32];
	struct tm *intm;
	int offset, hours, minutes;
	time_t t;

	offset = intime.offset;
	if (offset < 0) {
		sign = '-';
		offset = -offset;
	} else {
		sign = '+';
	}

	hours   = offset / 60;
	minutes = offset % 60;

	t = (time_t)intime.time + (intime.offset * 60);

	intm = gmtime(&t);
	strftime(out, sizeof(out), "%a %b %e %T %Y", intm);

	printf("%s%s %c%02d%02d\n", prefix, out, sign, hours, minutes);
}

void print_commit(const commit_wrapper& commit)
{
    // TODO: put in commit_wrapper ?
    char buf[GIT_OID_SHA1_HEXSIZE + 1];
    int i, count;

    git_oid_tostr(buf, sizeof(buf), &commit.oid());
    // TODO end

    signature_wrapper author = signature_wrapper::get_commit_author(commit);

    std::cout << "commit " << buf << std::endl;
    std::cout << "Author:\t" <<  author.name() << " " << author.email() << std::endl;
    print_time(author.when(), "Date:\t");
    std::cout << git_commit_message(commit) << "\n" << std::endl;
}

void log_subcommand::run()
{
    auto directory = get_current_git_path();
    auto bare = false;
    auto repo = repository_wrapper::init(directory, bare);
    // auto branch_name = repo.head().short_name();

    git_revwalk* walker;
    git_revwalk_new(&walker, repo);
    git_revwalk_push_head(walker);

    git_oid commit_oid;
    while (!git_revwalk_next(&commit_oid, walker))
    {
        commit_wrapper commit = repo.find_commit(commit_oid);
        print_commit(commit);
    }
}
