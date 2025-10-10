#include "ansi_code.hpp"

namespace ansi_code
{
    std::string cursor_to_row(size_t row)
    {
        return "\e[" + std::to_string(row) + "H";
    }

    bool is_down_arrow(std::string str)
    {
        return str == "\e[B" || str == "\e[1B]";
    }

    bool is_escape_char(char ch)
    {
        return ch == '\e';
    }

    bool is_up_arrow(std::string str)
    {
        return str == "\e[A" || str == "\e[1A]";
    }
}
