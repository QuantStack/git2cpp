#pragma once

#include <git2/remote.h>
#include <string_view>
#include <vector>

#include <git2.h>

#include "../wrapper/wrapper_base.hpp"

class remote_wrapper : public wrapper_base<git_remote>
{
public:

    using base_type = wrapper_base<git_remote>;

    ~remote_wrapper();

    remote_wrapper(remote_wrapper&&) = default;
    remote_wrapper& operator=(remote_wrapper&&) = default;

    std::string_view name() const;
    std::string_view url() const;
    std::string_view pushurl() const;

    std::vector<std::string> refspecs() const;

    void fetch(const git_strarray* refspecs, const git_fetch_options* opts, const char* reflog_message);
    void push(const git_strarray* refspecs, const git_push_options* opts);

private:

    explicit remote_wrapper(git_remote* remote);

    friend class repository_wrapper;
};
