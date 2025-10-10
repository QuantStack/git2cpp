#pragma once

#include <iostream>
#include "ansi_code.hpp"
#include "common.hpp"

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
