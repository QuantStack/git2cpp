#pragma once

#include <string>

/**
 * ANSI escape codes.
 * Use `termcolor` for colours.
 */
namespace ansi_code
{
    // Constants.
    const std::string bel = "\a";  // ASCII 7, used for audio/visual feedback.
    const std::string cursor_to_top = "\e[H";
    const std::string erase_screen = "\e[2J";

    const std::string enable_alternative_buffer = "\e[?1049h";
    const std::string disable_alternative_buffer = "\e[?1049l";

    const std::string hide_cursor = "\e[?25l";
    const std::string show_cursor = "\e[?25h";

    // Functions.
    std::string cursor_to_row(size_t row);

    bool is_escape_char(char ch);

    bool is_down_arrow(std::string str);
    bool is_up_arrow(std::string str);
}
