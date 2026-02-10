#pragma once

#include <exception>
#include <string>

enum class git2cpp_error_code
{
    GENERIC_ERROR = -1,
    FILESYSTEM_ERROR = 128,
    BAD_ARGUMENT = 129,
};

void throw_if_error(int exit_code);

class git_exception : public std::exception
{
public:

    git_exception(const std::string_view message, int error_code);
    git_exception(const std::string_view message, git2cpp_error_code error_code);

    int error_code() const;

    const char* what() const noexcept override;

private:

    std::string m_message;
    int m_error_code;
};
