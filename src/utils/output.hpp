#pragma once

#include <iostream>
#include "ansi_code.hpp"
#include "common.hpp"

// OS-specific libraries.
#include <termios.h>

// Scope object to hide the cursor. This avoids
// cursor twinkling when rewritting the same line
// too frequently.
struct cursor_hider : noncopyable_nonmovable
{
    cursor_hider()
    {
        std::cout << ansi_code::hide_cursor;
    }

    ~cursor_hider()
    {
        std::cout << ansi_code::show_cursor;
    }
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
