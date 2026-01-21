#pragma once

#include <git2.h>

#include "../wrapper/wrapper_base.hpp"
#include "../wrapper/diffstats_wrapper.hpp"

class diff_wrapper : public wrapper_base<git_diff>
{
public:

    using base_type = wrapper_base<git_diff>;

    ~diff_wrapper();

    diff_wrapper(diff_wrapper&&) noexcept = default;
    diff_wrapper& operator=(diff_wrapper&&) noexcept = default;

    void find_similar(git_diff_find_options* find_opts);
    void print(git_diff_format_t format, git_diff_line_cb print_cb, void* payload);
    diffstats_wrapper get_stats() const;
    static diff_wrapper diff_from_buffer(git_buf buf);


private:

    diff_wrapper(git_diff* diff);

    friend class buf_wrapper;
    friend class repository_wrapper;
};
