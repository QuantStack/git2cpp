#ifdef EMSCRIPTEN

#    include "subtransport.hpp"

#    include <regex>
#    include <sstream>

#    include <emscripten.h>
#    include <git2/sys/credential.h>
#    include <git2/sys/remote.h>

#    include "libgit2_internals.hpp"
#    include "stream.hpp"

// C functions.

static int wasm_http_action(
    git_smart_subtransport_stream** out,
    git_smart_subtransport* s,
    const char* url,
    git_smart_service_t action
)
{
    // An action is a single http/https request that is handled by a single wasm_http_stream.

    GIT_ASSERT_ARG(out);
    GIT_ASSERT_ARG(s);
    GIT_ASSERT_ARG(url);

    wasm_http_subtransport* subtransport = reinterpret_cast<wasm_http_subtransport*>(s);
    wasm_http_stream* stream = nullptr;
    *out = nullptr;

    auto service = select_service(action);
    if (!service.has_value())
    {
        git_error_set(0, "invalid http/https action");
        return -1;
    }

    if (subtransport->m_base_url.empty())
    {
        // Store base URL without trailing slashes.
        subtransport->m_base_url = std::regex_replace(url, std::regex("\\s+$"), "");
    }

    stream = new wasm_http_stream(subtransport, service.value());

    stream->m_parent.subtransport = &subtransport->m_parent;
    if (stream->m_service.m_method == GIT_HTTP_METHOD_GET)
    {
        stream->m_parent.read = wasm_http_stream_read;
    }
    else
    {
        stream->m_parent.write = wasm_http_stream_write;
        stream->m_parent.read = wasm_http_stream_read_response;
    }
    stream->m_parent.free = wasm_http_stream_free;
    *out = (git_smart_subtransport_stream*) stream;
    return 0;
}

static int wasm_http_close(git_smart_subtransport* s)
{
    return 0;
}

static void wasm_http_free(git_smart_subtransport* s)
{
    wasm_http_subtransport* subtransport = reinterpret_cast<wasm_http_subtransport*>(s);
    wasm_http_close(s);

    if (subtransport->m_credential != nullptr)
    {
        subtransport->m_credential->free(subtransport->m_credential);
    }

    delete subtransport;
}

int create_wasm_http_subtransport(git_smart_subtransport** out, git_transport* owner, void* param)
{
    GIT_ASSERT_ARG(out);
    GIT_ASSERT_ARG(owner);

    wasm_http_subtransport* subtransport = new wasm_http_subtransport();
    GIT_ASSERT_WITH_RETVAL(subtransport, -1);

    subtransport->m_parent.action = wasm_http_action;
    subtransport->m_parent.close = wasm_http_close;
    subtransport->m_parent.free = wasm_http_free;
    subtransport->m_owner = owner;
    subtransport->m_base_url = "";
    subtransport->m_credential = nullptr;

    *out = &subtransport->m_parent;
    return 0;
}

#endif  // EMSCRIPTEN
