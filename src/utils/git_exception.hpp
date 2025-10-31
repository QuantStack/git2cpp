#pragma once

#include <exception>
#include <string>

void throw_if_error(int exit_code);

class git_exception : public std::exception
{
public:

    git_exception(const std::string_view message, int error_code);

    int error_code() const;

    const char* what() const noexcept override;

private:

    std::string m_message;
    int m_error_code;
};
