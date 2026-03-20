#ifdef EMSCRIPTEN

#    include "response.hpp"

#    include "../utils/common.hpp"
#    include "libgit2_internals.hpp"

wasm_http_response::wasm_http_response(char* buffer, size_t buffer_size, size_t* bytes_read)
    : m_buffer(buffer)
    , m_buffer_size(buffer_size)
    , m_bytes_read(bytes_read)
    , m_status(0)
{
    *m_bytes_read = 0;
}

void wasm_http_response::add_header(const std::string& key, const std::string& value)
{
    m_response_headers.emplace(key, trim(value));
}

void wasm_http_response::clear()
{
    *m_bytes_read = 0;
    m_status = 0;
    m_status_text.clear();
    m_response_headers.clear();
}

std::optional<std::string> wasm_http_response::get_header(const std::string& key) const
{
    // Return the first header with the specified key.
    // If we ever have to handle multiple headers with the same key, will need to do something more
    // complicated here.
    auto header = m_response_headers.find(key);
    if (header != m_response_headers.end())
    {
        return header->second;
    }
    return std::nullopt;
}

bool wasm_http_response::has_header(const std::string& key) const
{
    return m_response_headers.find(key) != m_response_headers.end();
}

bool wasm_http_response::has_header_matches(const std::string& key, std::string_view match) const
{
    auto range = m_response_headers.equal_range(key);
    for (auto i = range.first; i != range.second; ++i)
    {
        if (i->second == match)
        {
            return true;
        }
    }
    return false;
}

bool wasm_http_response::has_header_starts_with(const std::string& key, std::string_view start) const
{
    auto range = m_response_headers.equal_range(key);
    for (auto i = range.first; i != range.second; ++i)
    {
        if (i->second.starts_with(start))
        {
            return true;
        }
    }
    return false;
}

#endif  // EMSCRIPTEN
