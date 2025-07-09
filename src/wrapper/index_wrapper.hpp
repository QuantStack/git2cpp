#pragma once

#include <git2.h>

#include "../utils/common.hpp"

class repository_wrapper;

class index_wrapper : public wrapper_base<git_index>
{
public:

    ~index_wrapper();

    index_wrapper(index_wrapper&&) = default;
    index_wrapper& operator=(index_wrapper&&) = default;

    static index_wrapper init(repository_wrapper& rw);

    void add_entry(const git_index_entry* entry);
    void add_all();


private:

    index_wrapper() = default;
};
