#include <git2.h>

#include "git_exception.hpp"

void throwIfError(int exitCode)
{
    if (exitCode < 0) {
        throw GitException(git_error_last()->message, exitCode);
    }
}


GitException::GitException(const std::string& message, int errorCode)
    : _message(message), _errorCode(errorCode)
{}

int GitException::errorCode() const
{
    return _errorCode;
}

const char* GitException::what() const noexcept
{
    return _message.c_str();
}
