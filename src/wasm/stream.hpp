#pragma once

#ifdef EMSCRIPTEN

#    include <git2/sys/transport.h>

#    include "libgit2_internals.hpp"
#    include "subtransport.hpp"

// A stream represents a single http/https request.
struct wasm_http_stream
{
    wasm_http_stream(wasm_http_subtransport* subtransport, http_service service);

    // Return true if URL is changed.
    bool ensure_final_url(const std::string final_url);

    // Return true if URL is changed from http to https.
    bool ensure_https();

    // Return full URL of request, which may have been modified to use CORS proxy.
    std::string get_full_url();

    git_smart_subtransport_stream m_parent;
    wasm_http_subtransport* m_subtransport;  // Not owned, needed for credentials, etc.
    http_service m_service;
    std::string m_unconverted_url;
    int m_request_index;
};

void wasm_http_stream_free(git_smart_subtransport_stream* s);

int wasm_http_stream_read(git_smart_subtransport_stream* s, char* buffer, size_t buffer_size, size_t* bytes_read);

int wasm_http_stream_read_response(
    git_smart_subtransport_stream* s,
    char* buffer,
    size_t buffer_size,
    size_t* bytes_read
);

int wasm_http_stream_write(git_smart_subtransport_stream* s, const char* buffer, size_t buffer_size);

#endif  // EMSCRIPTEN
