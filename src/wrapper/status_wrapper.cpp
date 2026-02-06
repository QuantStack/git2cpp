#include "../utils/git_exception.hpp"
#include "../wrapper/status_wrapper.hpp"

status_list_wrapper::~status_list_wrapper()
{
    git_status_list_free(p_resource);
    p_resource = nullptr;
}

status_list_wrapper status_list_wrapper::status_list(const repository_wrapper& rw)
{
    status_list_wrapper res;
    throw_if_error(git_status_list_new(&(res.p_resource), rw, nullptr));

    std::size_t status_list_size = git_status_list_entrycount(res.p_resource);
    for (std::size_t i = 0; i < status_list_size; ++i)
    {
        auto entry = git_status_byindex(res.p_resource, i);
        res.m_entries[entry->status].push_back(entry);
    }

    if (!res.get_entry_list(GIT_STATUS_INDEX_NEW).empty() || !res.get_entry_list(GIT_STATUS_INDEX_MODIFIED).empty() || !res.get_entry_list(GIT_STATUS_INDEX_DELETED).empty() || !res.get_entry_list(GIT_STATUS_INDEX_RENAMED).empty() || !res.get_entry_list(GIT_STATUS_INDEX_TYPECHANGE).empty())
    {
        res.m_tobecommited_header_flag = true;
    }
    if (!res.get_entry_list(GIT_STATUS_WT_NEW).empty())
    {
        res.m_untracked_header_flag = true;
    }
    if (!res.get_entry_list(GIT_STATUS_WT_MODIFIED).empty() || !res.get_entry_list(GIT_STATUS_WT_DELETED).empty() || !res.get_entry_list(GIT_STATUS_WT_TYPECHANGE).empty() || !res.get_entry_list(GIT_STATUS_WT_RENAMED).empty())
    {
        res.m_notstagged_header_flag = true;
    }
    if (!res.get_entry_list(GIT_STATUS_IGNORED).empty())
    {
        res.m_ignored_header_flag = true;
    }
    if (!res.get_entry_list(GIT_STATUS_CONFLICTED).empty())
    {
        res.m_unmerged_header_flag = true;
    }
    // if (!res.tobecommited_header_flag)
    // {
    //     res.m_nothingtocommit_message_flag = true;
    // }

    return res;
}

bool status_list_wrapper::has_untracked_header() const
{
    return m_untracked_header_flag;
}
bool status_list_wrapper::has_tobecommited_header() const
{
    return m_tobecommited_header_flag;
}
bool status_list_wrapper::has_ignored_header() const
{
    return m_ignored_header_flag;
}
bool status_list_wrapper::has_notstagged_header() const
{
    return m_notstagged_header_flag;
}
bool status_list_wrapper::has_unmerged_header() const
{
    return m_unmerged_header_flag;
}
bool status_list_wrapper::has_nothingtocommit_message() const
{
    return m_nothingtocommit_message_flag;
}

auto status_list_wrapper::get_entry_list(git_status_t status) const -> const status_entry_list&
{
    if (auto search = m_entries.find(status); search != m_entries.end())
    {
        return search->second;
    }
    else
    {
        return m_empty;
    }
}

