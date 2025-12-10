#include <vector>
#include <string>

#include <git2/remote.h>

#include "../utils/git_exception.hpp"
#include "../wrapper/remote_wrapper.hpp"

remote_wrapper::remote_wrapper(git_remote* remote)
    : base_type(remote)
{
}

remote_wrapper::~remote_wrapper()
{
    git_remote_free(p_resource);
    p_resource = nullptr;
}

std::string_view remote_wrapper::name() const
{
    const char* out = git_remote_name(*this);
    return out ? std::string_view(out) : std::string_view();
}

std::string_view remote_wrapper::url() const
{
    const char* out = git_remote_url(*this);
    return out ? std::string_view(out) : std::string_view();
}

std::string_view remote_wrapper::pushurl() const
{
    const char* out = git_remote_pushurl(*this);
    return out ? std::string_view(out) : std::string_view();
}

std::vector<std::string> remote_wrapper::refspecs() const
{
    git_strarray refspecs = {0};
    std::vector<std::string> result;

    if (git_remote_get_fetch_refspecs(&refspecs, *this) == 0)
    {
        for (size_t i = 0; i < refspecs.count; ++i)
        {
            result.emplace_back(refspecs.strings[i]);
        }
        git_strarray_dispose(&refspecs);
    }

    return result;
}

void remote_wrapper::fetch(const git_strarray* refspecs, const git_fetch_options* opts, const char* reflog_message)
{
    throw_if_error(git_remote_fetch(*this, refspecs, opts, reflog_message));
}

void remote_wrapper::push(const git_strarray* refspecs, const git_push_options* opts)
{
    throw_if_error(git_remote_push(*this, refspecs, opts));
}
