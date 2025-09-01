#pragma once

#include <utility>
#include <string_view>

#include <git2.h>

#include "../wrapper/wrapper_base.hpp"

class commit_wrapper;
class repository_wrapper;

class signature_wrapper : public wrapper_base<git_signature>
{
public:

    using author_committer_signatures = std::pair<signature_wrapper, signature_wrapper>;

    ~signature_wrapper();

    signature_wrapper(signature_wrapper&&) noexcept = default;
    signature_wrapper& operator=(signature_wrapper&&) noexcept = default;

    std::string_view name() const;
    std::string_view email() const;
    git_time when() const;

    static author_committer_signatures get_default_signature_from_env(repository_wrapper&);
    static signature_wrapper get_commit_author(const commit_wrapper&);
    static signature_wrapper get_commit_committer(const commit_wrapper&);
    static signature_wrapper signature_now(std::string name, std::string email);
    static author_committer_signatures signature_now(std::string author_name, std::string autor_email, std::string committer_name, std::string committer_email);

private:

    signature_wrapper() = default;
    bool m_ownership=true;
};
