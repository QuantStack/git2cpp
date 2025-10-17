#include "output.hpp"

// OS-specific libraries.
#include <sys/ioctl.h>

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
