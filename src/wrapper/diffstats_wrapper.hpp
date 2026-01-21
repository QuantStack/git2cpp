#pragma once

#include <git2.h>

#include "../wrapper/wrapper_base.hpp"

class diffstats_wrapper : public wrapper_base<git_diff_stats>
{
public:

    using base_type = wrapper_base<git_diff_stats>;

    ~diffstats_wrapper();

    diffstats_wrapper(diffstats_wrapper&&) noexcept = default;
    diffstats_wrapper& operator=(diffstats_wrapper&&) noexcept = default;

    git_buf to_buf(git_diff_stats_format_t format, size_t width);

private:

    diffstats_wrapper(git_diff_stats* stats);

    friend class diff_wrapper;
};
