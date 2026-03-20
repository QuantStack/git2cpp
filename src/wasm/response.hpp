#pragma once

#ifdef EMSCRIPTEN

#    include <map>
#    include <optional>
#    include <string>

// Response from a http(s) request.
// The lifetimes of the buffer, buffer_size and bytes_read are managed by libgit2, we just fill them
// with the data received from the request.
class wasm_http_response
{
public:

    wasm_http_response(char* buffer, size_t buffer_size, size_t* bytes_read);

    void add_header(const std::string& key, const std::string& value);

    void clear();

    std::optional<std::string> get_header(const std::string& key) const;

    bool has_header(const std::string& key) const;

    bool has_header_matches(const std::string& key, std::string_view match) const;

    bool has_header_starts_with(const std::string& key, std::string_view start) const;

    char* m_buffer;  // Not owned.
    size_t m_buffer_size;
    size_t* m_bytes_read;  // Not owned.
    int32_t m_status;      // Specific type corresponding to i32 in emscripten setValue call.
    std::string m_status_text;

private:

    // Support multiple headers with the same key.
    std::multimap<const std::string, const std::string> m_response_headers;
};

#endif  // EMSCRIPTEN
