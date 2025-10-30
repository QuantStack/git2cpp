#pragma once

#include <map>
#include <vector>

#include <git2.h>

#include "../wrapper/repository_wrapper.hpp"
#include "../wrapper/wrapper_base.hpp"

class status_list_wrapper : public wrapper_base<git_status_list>
{
public:
    using status_entry_list = std::vector<const git_status_entry*>;

    ~status_list_wrapper();

    status_list_wrapper(status_list_wrapper&&) noexcept = default;
    status_list_wrapper& operator=(status_list_wrapper&&) noexcept = default;

    static status_list_wrapper status_list(const repository_wrapper& wrapper);

    const status_entry_list& get_entry_list(git_status_t status) const;

    bool has_untracked_header() const;
    bool has_tobecommited_header() const;
    bool has_ignored_header() const;
    bool has_notstagged_header() const;
    bool has_unmerged_header() const;
    bool has_nothingtocommit_message() const;

private:

    status_list_wrapper() = default;

    using status_entry_map = std::map<git_status_t, status_entry_list>;
    status_entry_map m_entries;
    status_entry_list m_empty = {};
    bool m_untracked_header_flag = false;
    bool m_tobecommited_header_flag = false;
    bool m_ignored_header_flag = false;
    bool m_notstagged_header_flag = false;
    bool m_unmerged_header_flag = false;
    bool m_nothingtocommit_message_flag = false;
};
