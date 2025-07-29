#pragma once

#include <utility>

#include <git2.h>

#include "../wrapper/wrapper_base.hpp"

class repository_wrapper;

class signature_wrapper : public wrapper_base<git_signature>
{
public:
    using author_committer_signatures = std::pair<signature_wrapper, signature_wrapper>;

    ~signature_wrapper();

    signature_wrapper(signature_wrapper&&) = default;
    signature_wrapper& operator=(signature_wrapper&&) = default;

    static author_committer_signatures get_default_signature_from_env(repository_wrapper&);

private:

    signature_wrapper() = default;
};
