#include "revlist_subcommand.hpp"
#include "../wrapper/repository_wrapper.hpp"
#include "../wrapper/revwalk_wrapper.hpp"

revlist_subcommand::revlist_subcommand(const libgit2_object&, CLI::App& app)
{
    auto* sub = app.add_subcommand("rev-list", "Lists commit objects in reverse chronological order");

    sub->add_option("<commit>", m_commit, "");
    sub->add_option("-n,--max-count", m_max_count_flag, "Limit the output to <number> commits.");

    sub->callback([this]() { this->run(); });
}

void revlist_subcommand::run()
{
    if (m_commit.empty())
    {
        throw std::runtime_error("usage: git rev-list [<options>] <commit>... [--] [<path>...]");   // TODO: add  help info
    }

    auto directory = get_current_git_path();
    auto repo = repository_wrapper::open(directory);
    git_oid start_commit_oid;
    int not_sha1 = git_oid_fromstrp(&start_commit_oid, m_commit.c_str());
    if (not_sha1)
    {
        commit_wrapper start_commit = repo.find_commit(m_commit);
        start_commit_oid = start_commit.oid();
    }

    revwalk_wrapper walker = repo.new_walker();
    walker.push(start_commit_oid);

    std::size_t i=0;
    git_oid commit_oid;
    char buf[GIT_OID_SHA1_HEXSIZE + 1];
    while (!walker.next(commit_oid) && i<m_max_count_flag)
    {
        git_oid_fmt(buf, &commit_oid);
		buf[GIT_OID_SHA1_HEXSIZE] = '\0';
		std::cout << buf << std::endl;
		++i;
    }
}
