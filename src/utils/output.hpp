#pragma once

#include <iostream>
#include "common.hpp"

// Scope object to hide the cursor. This avoids
// cursor twinkling when rewritting the same line
// too frequently.
struct cursor_hider : noncopyable_nonmovable
{
    cursor_hider()
    {
        std::cout << "\e[?25l";
    }

    ~cursor_hider()
    {
        std::cout << "\e[?25h";
    }
};
