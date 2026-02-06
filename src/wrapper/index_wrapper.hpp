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

    void add_entry(const std::string& path);
    void add_entries(std::vector<std::string> patterns);
    void add_all();

    void remove_entry(const std::string& path);
    void remove_entries(std::vector<std::string> paths);
    void remove_directories(std::vector<std::string> paths);

    bool has_conflict() const;
    void output_conflicts();
    void conflict_cleanup();


private:

    index_wrapper() = default;
    void add_impl(std::vector<std::string> patterns);

    git_index_conflict_iterator* create_conflict_iterator();
};
