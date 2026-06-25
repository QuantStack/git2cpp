#include "strarray_wrapper.hpp"

strarray_owned_wrapper::strarray_owned_wrapper()
    : m_array{nullptr, 0}
{
}

strarray_owned_wrapper::strarray_owned_wrapper(git_strarray&& arr)
    : m_array(std::move(arr))
{
}

strarray_owned_wrapper::strarray_owned_wrapper(strarray_owned_wrapper&& rhs)
    : m_array(std::move(rhs.m_array))
{
    rhs.m_array = git_strarray{nullptr, 0};
}

strarray_owned_wrapper& strarray_owned_wrapper::operator=(strarray_owned_wrapper&& rhs)
{
    std::swap(m_array.strings, rhs.m_array.strings);
    std::swap(m_array.count, rhs.m_array.count);
    return *this;
}

strarray_owned_wrapper::~strarray_owned_wrapper()
{
    git_strarray_dispose(&m_array);
}

strarray_owned_wrapper::operator git_strarray*()
{
    return &m_array;
}

size_t strarray_owned_wrapper::size() const
{
    return m_array.count;
}

std::string strarray_owned_wrapper::operator[](size_t i) const
{
    return {m_array.strings[i]};
}

strarray_view_wrapper::strarray_view_wrapper(std::vector<std::string> patterns)
    : m_patterns(std::move(patterns))
{
    init_str_array();
}

strarray_view_wrapper::strarray_view_wrapper(strarray_view_wrapper&& rhs)
    : m_patterns(std::move(rhs.m_patterns))
{
    init_str_array();
    rhs.reset_str_array();
}

strarray_view_wrapper& strarray_view_wrapper::operator=(strarray_view_wrapper&& rhs)
{
    using std::swap;
    swap(m_patterns, rhs.m_patterns);
    swap(m_array.strings, rhs.m_array.strings);
    swap(m_array.count, rhs.m_array.count);
    return *this;
}

strarray_view_wrapper::~strarray_view_wrapper()
{
    reset_str_array();
}

strarray_view_wrapper::operator git_strarray*()
{
    return &m_array;
}

void strarray_view_wrapper::reset_str_array()
{
    delete[] m_array.strings;
    m_array = {nullptr, 0};
}

void strarray_view_wrapper::init_str_array()
{
    m_array.strings = new char*[m_patterns.size()];
    m_array.count = m_patterns.size();
    for (size_t i = 0; i < m_patterns.size(); ++i)
    {
        m_array.strings[i] = const_cast<char*>(m_patterns[i].c_str());
    }
}

size_t strarray_view_wrapper::size() const
{
    return m_patterns.size();
}
