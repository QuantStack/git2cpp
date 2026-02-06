#include <git2/index.h>
#include <iostream>
#include <vector>

#include "index_wrapper.hpp"
#include "../utils/common.hpp"
#include "../utils/git_exception.hpp"
#include "../wrapper/repository_wrapper.hpp"


index_wrapper::~index_wrapper()
{
    git_index_free(p_resource);
    p_resource=nullptr;
}

index_wrapper index_wrapper::init(repository_wrapper& rw)
{
    index_wrapper index;
    throw_if_error(git_repository_index(&(index.p_resource), rw));
    return index;
}

void index_wrapper::add_entry(const std::string& path)
{
    throw_if_error(git_index_add_bypath(*this, path.c_str()));
}

void index_wrapper::add_entries(std::vector<std::string> patterns)
{
    add_impl(std::move(patterns));
}

void index_wrapper::add_all()
{
    add_impl({{"."}});
}

void index_wrapper::add_impl(std::vector<std::string> patterns)
{
    git_strarray_wrapper array{patterns};
    throw_if_error(git_index_add_all(*this, array, 0, NULL, NULL));
}

void index_wrapper::remove_entry(const std::string& path)
{
    throw_if_error(git_index_remove_bypath(*this, path.c_str()));
}

void index_wrapper::write()
{
    throw_if_error(git_index_write(*this));
}

git_oid index_wrapper::write_tree()
{
    git_oid tree_id;
    throw_if_error(git_index_write_tree(&tree_id, *this));
    return tree_id;
}

bool index_wrapper::has_conflict() const
{
    return git_index_has_conflicts(*this);
}

git_index_conflict_iterator* index_wrapper::create_conflict_iterator()
{
    git_index_conflict_iterator* conflict_iterator;
    throw_if_error(git_index_conflict_iterator_new(&conflict_iterator, *this));
    return conflict_iterator;
}

void index_wrapper::output_conflicts()
{
	git_index_conflict_iterator* conflicts = create_conflict_iterator();

	const git_index_entry* ancestor;
	const git_index_entry* our;
	const git_index_entry* their;
	int err = 0;
	std::string msg_conflict;

	while ((err = git_index_conflict_next(&ancestor, &our, &their, conflicts)) == 0)
	{
	    std::string ancestor_path = ancestor ? ancestor->path : "";
		std::string our_path = our->path ? our->path : "NULL";
	    std::string their_path = their->path ? their->path : "NULL";
		msg_conflict  = "conflict: " + ancestor_path + " " + our_path + " " + their_path;
		std::cout << msg_conflict << std::endl;
// Message with git is a bit different:
// Auto-merging mook_file.txt
// CONFLICT (add/add): Merge conflict in mook_file.txt
// Automatic merge failed; fix conflicts and then commit the result.
	}

	if (err != GIT_ITEROVER)
	{
		std::cout << "error iterating conflicts" << std::endl;
	}

	git_index_conflict_iterator_free(conflicts);
}

void index_wrapper::conflict_cleanup()
{
    throw_if_error(git_index_conflict_cleanup(*this));
}
