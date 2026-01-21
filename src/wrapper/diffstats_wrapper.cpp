#include "../utils/git_exception.hpp"
#include "../wrapper/diffstats_wrapper.hpp"

diffstats_wrapper::diffstats_wrapper(git_diff_stats* stats)
    : base_type(stats)
{
}

diffstats_wrapper::~diffstats_wrapper()
{
    git_diff_stats_free(p_resource);
    p_resource = nullptr;
}

git_buf diffstats_wrapper::to_buf(git_diff_stats_format_t format, size_t width)
{
    git_buf buf = GIT_BUF_INIT;
    throw_if_error(git_diff_stats_to_buf(&buf, *this, format, 80));
    return buf;
}
