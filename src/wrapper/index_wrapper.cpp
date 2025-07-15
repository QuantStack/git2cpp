#include "index_wrapper.hpp"
#include "../utils/git_exception.hpp"
#include "../wrapper/repository_wrapper.hpp"

#include <iostream>

index_wrapper::~index_wrapper()
{
    git_index_free(p_resource);
    p_resource=nullptr;
}

index_wrapper index_wrapper::init(repository_wrapper& rw)
{
    index_wrapper index;
    throwIfError(git_repository_index(&(index.p_resource), rw));
    return index;
}

void index_wrapper::add_entry(const git_index_entry* entry)
{
    throwIfError(git_index_add(*this, entry));
}

void index_wrapper::add_all()
{
    const char* patterns[] = {"."};
    git_strarray array{(char**)patterns, 1};
    throwIfError(git_index_add_all(*this, &array, 0, NULL, NULL));
    throwIfError(git_index_write(*this));
}
