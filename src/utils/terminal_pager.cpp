#include <cctype>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <ranges>

// OS-specific libraries.
#include <sys/ioctl.h>

#include <termcolor/termcolor.hpp>

#include "ansi_code.hpp"
#include "output.hpp"
#include "terminal_pager.hpp"
#include "common.hpp"

terminal_pager::terminal_pager()
    : m_rows(0), m_columns(0), m_start_row_index(0)
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

    if (ansi_code::is_escape_char(ch))  // Start of ANSI escape sequence.
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
    if (termcolor::_internal::is_atty(std::cout))
    {
        // Should we do anything with cerr?
        m_cout_rdbuf = std::cout.rdbuf(&m_stringbuf);
    }
    else
    {
        m_cout_rdbuf = std::cout.rdbuf();
    }
}

bool terminal_pager::process_input(std::string input)
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
            if (ansi_code::is_up_arrow(input))
            {
                scroll(true, false);  // Up a line.
                return false;
            }
            else if (ansi_code::is_down_arrow(input))
            {
                scroll(false, false);  // Down a line.
                return false;
            }
    }

    std::cout << ansi_code::bel;
    return false;
}

void terminal_pager::release_cout()
{
    std::cout.rdbuf(m_cout_rdbuf);
}

void terminal_pager::render_terminal() const
{
    auto end_row_index = m_start_row_index + m_rows - 1;

    std::cout << ansi_code::erase_screen;
    std::cout << ansi_code::cursor_to_top;

    for (size_t i = m_start_row_index; i < end_row_index; i++)
    {
        if (i >= m_lines.size())
        {
            break;
        }
        std::cout << m_lines[i] << std::endl;
    }

    std::cout << ansi_code::cursor_to_row(m_rows);  // Move cursor to bottom row of terminal.
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
        std::cout << ansi_code::bel;
    }
    else
    {
        render_terminal();
    }
}

void terminal_pager::show()
{
    release_cout();

    m_lines = split_input_at_newlines(m_stringbuf.view());

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

    alternative_buffer alt_buffer;

    m_start_row_index = 0;
    render_terminal();

    bool stop = false;
    do
    {
        stop = process_input(get_input());
    } while (!stop);

    m_lines.clear();
    m_start_row_index = 0;
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
