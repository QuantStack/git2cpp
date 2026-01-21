#include "../utils/git_exception.hpp"
#include "../wrapper/diff_wrapper.hpp"

diff_wrapper::diff_wrapper(git_diff* diff)
    : base_type(diff)
{
}

diff_wrapper::~diff_wrapper()
{
    git_diff_free(p_resource);
    p_resource = nullptr;
}

void diff_wrapper::find_similar(git_diff_find_options* find_opts)
{
    throw_if_error(git_diff_find_similar(p_resource, find_opts));
}

void diff_wrapper::print(git_diff_format_t format, git_diff_line_cb print_cb, void* payload)
{
    throw_if_error(git_diff_print(p_resource, format, print_cb, payload));
}

diffstats_wrapper diff_wrapper::get_stats() const
{
    git_diff_stats* stats;
    throw_if_error(git_diff_get_stats(&stats, *this));
    return diffstats_wrapper(stats);
}

diff_wrapper diff_wrapper::diff_from_buffer(git_buf buf)
{
    git_diff* diff;
    throw_if_error(git_diff_from_buffer(&diff, buf.ptr, buf.size));
    return diff_wrapper(diff);
}
