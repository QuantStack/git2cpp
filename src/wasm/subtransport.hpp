#pragma once

#ifdef EMSCRIPTEN

#    include <string>

#    include <git2/sys/transport.h>

// A single wasm_http_subtransport manages all http(s) requests of a single git2cpp command call.
// Each request has its own wasm_http_stream, here we store extra information that needs to be
// reused by subsequent requests.
struct wasm_http_subtransport
{
    git_smart_subtransport m_parent;
    git_transport* m_owner;  // Not owned.

    // Data stored for reuse on other streams of this transport:
    std::string m_base_url;
    std::string m_authorization_header;
    git_credential* m_credential;        // libgit2 creates this, we are responsible for deleting it.
    unsigned long m_request_timeout_ms;  // Timeout for http(s) requests in milliseconds.
};

// git_smart_subtransport_cb
int create_wasm_http_subtransport(git_smart_subtransport** out, git_transport* owner, void* param);

#endif  // EMSCRIPTEN
