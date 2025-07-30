#pragma once

#include <string>
#include <vector>

#include <git2.h>

#include "../wrapper/wrapper_base.hpp"

class repository_wrapper;

class index_wrapper : public wrapper_base<git_index>
{
public:

    ~index_wrapper();

    index_wrapper(index_wrapper&&) noexcept = default;
    index_wrapper& operator=(index_wrapper&&) noexcept = default;

    static index_wrapper init(repository_wrapper& rw);

    void write();
    git_oid write_tree();

    void add_entries(std::vector<std::string> patterns);
    void add_all();


private:

    index_wrapper() = default;
    void add_impl(std::vector<std::string> patterns);
};
