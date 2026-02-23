#include "ansi_code.hpp"
#include "input_output.hpp"

// OS-specific libraries.
#include <sys/ioctl.h>

cursor_hider::cursor_hider(bool hide /* = true */)
    : m_hide(hide)
{
    std::cout << (m_hide ? ansi_code::hide_cursor : ansi_code::show_cursor);
}

cursor_hider::~cursor_hider()
{
    std::cout << (m_hide ? ansi_code::show_cursor :  ansi_code::hide_cursor);
}


alternative_buffer::alternative_buffer()
{
    tcgetattr(fileno(stdin), &m_previous_termios);
    auto new_termios = m_previous_termios;
    // Disable canonical mode (buffered I/O) and echo from stdin to stdout.
    new_termios.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(fileno(stdin), TCSANOW, &new_termios);

    std::cout << ansi_code::enable_alternative_buffer;
}

alternative_buffer::~alternative_buffer()
{
    std::cout << ansi_code::disable_alternative_buffer;

    // Restore previous termios settings.
    tcsetattr(fileno(stdin), TCSANOW, &m_previous_termios);
}

echo_control::echo_control(bool echo)
    : m_echo(echo)
{
    if (!m_echo) {
        tcgetattr(fileno(stdin), &m_previous_termios);
        auto new_termios = m_previous_termios;
        new_termios.c_lflag &= ~ECHO;
        tcsetattr(fileno(stdin), TCSANOW, &new_termios);
    }
}

echo_control::~echo_control()
{
    if (!m_echo) {
        // Restore previous termios settings.
        tcsetattr(fileno(stdin), TCSANOW, &m_previous_termios);
    }
}


std::string prompt_input(const std::string_view prompt, bool echo /* = true */)
{
    std::cout << prompt;

    echo_control ec(echo);
    std::string input;

    cursor_hider ch(false);  // Re-enable cursor if currently hidden.
    std::getline(std::cin, input);

    if (!echo) {
        std::cout << std::endl;
    }

    // Maybe sanitise input, removing escape codes?
    return input;
}
