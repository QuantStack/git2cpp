#include <git2.h>

#include "git_exception.hpp"

void throw_if_error(int exit_code)
{
    if (exit_code < 0)
    {
        throw git_exception("error: " + std::string(git_error_last()->message), exit_code);
    }
}


git_exception::git_exception(const std::string_view message, int error_code)
    : m_message(message), m_error_code(error_code)
{}

int git_exception::error_code() const
{
    return m_error_code;
}

const char* git_exception::what() const noexcept
{
    return m_message.c_str();
}
