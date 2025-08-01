#include "index_wrapper.hpp"
#include "../utils/common.hpp"
#include "../utils/git_exception.hpp"
#include "../wrapper/repository_wrapper.hpp"

#include <vector>

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
    // throw_if_error(git_index_write(*this));
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
