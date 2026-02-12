#pragma once

#include <vector>
#include <sstream>

/**
 * Terminal pager that displays output written to stdout one page at a time, allowing the user to
 * interactively scroll up and down. If cout is not a tty or the output is shorter than a single
 * terminal page it does nothing.
 *
 * It expects all of cout to be written before the first page is displayed, so it does not pipe from
 * cout which would be a more complicated implementation allowing the first page to be displayed
 * before all of the output is written. This may need to be reconsidered if we need more performant
 * handling of slow subcommands such as `git2cpp log` of repos with long histories.
 *
 * Keys handled:
 *     d, space                   scroll down a page
 *     u                          scroll up a page
 *     q                          quit pager
 *     down arrow, enter, return  scroll down a line
 *     up arrow                   scroll up a line
 *
 * Emits a BEL (ASCII 7) for unrecognised keys or attempts to scroll too far, which is used by some
 * terminals for visual and/or audible feedback.
 *
 * Does not respond to a change of terminal size whilst it is waiting for input, but it will the
 * next time the output is scrolled.
 */
class terminal_pager
{
public:
    terminal_pager();

    ~terminal_pager();

    void show();

private:
    std::string get_input() const;

    void maybe_grab_cout();

    // Return true if should stop pager.
    bool process_input(std::string input);

    void release_cout();

    void render_terminal() const;

    void scroll(bool up, bool page);

    void update_terminal_size();


    std::stringbuf m_stringbuf;
    std::streambuf* m_cout_rdbuf;
    std::vector<std::string> m_lines;
    size_t m_rows, m_columns;
    size_t m_start_row_index;
};
