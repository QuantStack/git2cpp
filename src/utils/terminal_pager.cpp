#include <cctype>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <ranges>

// OS-specific libraries.
#include <sys/ioctl.h>
#include <termios.h>

#include <termcolor/termcolor.hpp>

#include "terminal_pager.hpp"

terminal_pager::terminal_pager()
    : m_grabbed(false), m_rows(0), m_columns(0), m_start_row_index(0)
{
    maybe_grab_cout();
}

terminal_pager::~terminal_pager()
{
    release_cout();
}

std::string terminal_pager::get_input() const
{
    // Blocks until input received.
    std::string str;
    char ch;
    std::cin.get(ch);
    str += ch;

    if (ch == '\e')  // Start of ANSI escape sequence.
    {
        do
        {
            std::cin.get(ch);
            str += ch;
        } while (!std::isalpha(ch));  // ANSI escape sequence ends with a letter.
    }

    return str;
}

void terminal_pager::maybe_grab_cout()
{
    // Unfortunately need to access _internal namespace of termcolor to check if a tty.
    if (!m_grabbed && termcolor::_internal::is_atty(std::cout))
    {
        // Should we do anything with cerr?
        m_cout_rdbuf = std::cout.rdbuf(m_oss.rdbuf());
        m_grabbed = true;
    }
}

bool terminal_pager::process_input(const std::string& input)
{
    if (input.size() == 0)
    {
        return true;
    }

    switch (input[0])
    {
        case 'q':
        case 'Q':
            return true;  // Exit pager.
        case 'u':
        case 'U':
            scroll(true, true);  // Up a page.
            return false;
        case 'd':
        case 'D':
        case ' ':
            scroll(false, true);  // Down a page.
            return false;
        case '\n':
            scroll(false, false);  // Down a line.
            return false;
        case '\e':  // ANSI escape sequence.
            // Cannot switch on a std::string.
            if (input == "\e[A" || input == "\e[1A]")  // Up arrow.
            {
                scroll(true, false);  // Up a line.
                return false;
            }
            else if (input == "\e[B" || input == "\e[1B]")  // Down arrow.
            {
                scroll(false, false);  // Down a line.
                return false;
            }
    }

    std::cout << '\a';  // Emit BEL for visual feedback.
    return false;
}

void terminal_pager::release_cout()
{
    if (m_grabbed)
    {
        std::cout.rdbuf(m_cout_rdbuf);
        m_grabbed = false;
    }
}

void terminal_pager::render_terminal() const
{
    auto end_row_index = m_start_row_index + m_rows - 1;

    std::cout << "\e[2J";  // Erase screen.
    std::cout << "\e[H";  // Cursor to top.

    for (size_t i = m_start_row_index; i < end_row_index; i++)
    {
        if (i >= m_lines.size())
        {
            break;
        }
        std::cout << m_lines[i] << std::endl;
    }

    std::cout << "\e[" << m_rows << "H";  // Move cursor to bottom row of terminal.
    std::cout << ":";
}

void terminal_pager::scroll(bool up, bool page)
{
    update_terminal_size();
    const auto old_start_row_index = m_start_row_index;
    size_t offset = page ? m_rows - 1 : 1;

    if (up)
    {
        // Care needed to avoid underflow of unsigned size_t.
        if (m_start_row_index >= offset)
        {
            m_start_row_index -= offset;
        }
        else
        {
            m_start_row_index = 0;
        }
    }
    else
    {
        m_start_row_index += offset;
        auto end_row_index = m_start_row_index + m_rows - 1;
        if (end_row_index > m_lines.size())
        {
            m_start_row_index = m_lines.size() - (m_rows - 1);
        }
    }

    if (m_start_row_index == old_start_row_index)
    {
        // No change, emit BEL for visual feedback.
        std::cout << '\a';
    }
    else
    {
        render_terminal();
    }
}

void terminal_pager::show()
{
    if (!m_grabbed)
    {
        return;
    }

    release_cout();

    split_input_at_newlines(m_oss.view());

    update_terminal_size();
    if (m_rows == 0 || m_lines.size() <= m_rows - 1)
    {
        // Don't need to use pager, can display directly.
        for (auto line : m_lines)
        {
            std::cout << line << std::endl;
        }
        m_lines.clear();
        return;
    }

    struct termios old_termios;
    tcgetattr(fileno(stdin), &old_termios);
    auto new_termios = old_termios;
    // Disable canonical mode (buffered I/O) and echo from stdin to stdout.
    new_termios.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(fileno(stdin), TCSANOW, &new_termios);

    std::cout << "\e[?1049h";  // Enable alternative buffer.

    m_start_row_index = 0;
    render_terminal();

    bool stop = false;
    do
    {
        stop = process_input(get_input());
    } while (!stop);

    std::cout << "\e[?1049l";  // Disable alternative buffer.

    // Restore original termios settings.
    tcsetattr(fileno(stdin), TCSANOW, &old_termios);

    m_lines.clear();
    m_start_row_index = 0;
}

void terminal_pager::split_input_at_newlines(std::string_view str)
{
    auto split = str | std::ranges::views::split('\n')
                     | std::ranges::views::transform([](auto&& range) {
                           return std::string(range.begin(), std::ranges::distance(range));
                       });
    m_lines = std::vector<std::string>{split.begin(), split.end()};
}

void terminal_pager::update_terminal_size()
{
    struct winsize size;
    int err = ioctl(fileno(stdout), TIOCGWINSZ, &size);
    if (err == 0)
    {
        m_rows = size.ws_row;
        m_columns = size.ws_col;
    }
    else
    {
        m_rows = m_columns = 0;
    }
}
