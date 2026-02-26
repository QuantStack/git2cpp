#pragma once

#include <iostream>
#include "common.hpp"

// OS-specific libraries.
#include <termios.h>

// Scope object to hide the cursor. This avoids
// cursor twinkling when rewritting the same line
// too frequently.
// If you are within a cursor_hider context you can
// reenable the cursor using cursor_hider(false).
class cursor_hider : noncopyable_nonmovable
{
public:
    cursor_hider(bool hide = true);

    ~cursor_hider();

private:
    bool m_hide;
};

// Scope object to use alternative output buffer for
// fullscreen interactive terminal input/output.
class alternative_buffer : noncopyable_nonmovable
{
public:
    alternative_buffer();

    ~alternative_buffer();

private:
    struct termios m_previous_termios;
};

// Scope object to control echo of stdin to stdout.
// This should be disabled when entering passwords for example.
class echo_control : noncopyable_nonmovable
{
public:
    echo_control(bool echo);

    ~echo_control();

private:
    bool m_echo;
    struct termios m_previous_termios;
};

// Display a prompt on stdout and return newline-terminated input received on
// stdin from the user.  The `echo` argument controls whether stdin is echoed
// to stdout, use `false` for passwords.
std::string prompt_input(const std::string_view prompt, bool echo = true);
