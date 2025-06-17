#pragma once

#include <exception>
#include <string>

void throwIfError(int exitCode);

class GitException : public std::exception
{
public:
    GitException(const std::string& message, int errorCode);

    int errorCode() const;

    const char* what() const noexcept override;

private:
    std::string _message;
    int _errorCode;
};
