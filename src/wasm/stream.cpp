#ifdef EMSCRIPTEN

#    include "stream.hpp"

#    include <regex>
#    include <sstream>

#    include <emscripten.h>

#    include "../utils/common.hpp"
#    include "response.hpp"

// Buffer size used in transport_smart, hardcoded in libgit2.
#    define EMFORGE_BUFSIZE 65536

// JavasScript functions.

EM_JS(const char*, js_base64_encode, (const char* input), {
    const input_js = UTF8ToString(input);
    const encoded = btoa(input_js);
    return stringToNewUTF8(encoded);
});

EM_JS(void, js_delete_request, (int request_index), {
    const cache = Module["git2cpp_js_cache"];
    if (Object.hasOwn(cache, request_index))
    {
        delete cache[request_index];
    }
});

// Return the latest error string set in JS.  Caller must delete the returned const char*.
EM_JS(const char*, js_get_error, (void), {
    const err = Module["git2cpp_js_error"];
    if (!err)
    {
        return stringToNewUTF8("unknown error");
    }
    let msg = err.message;
    if (err.name)
    {
        msg = err.name + ": " + msg;
    }
    return stringToNewUTF8(msg);
});

EM_JS(
    int,
    js_request,
    (const char* url,
     const char* method,
     const char* content_type_header,
     const char* authorization_header,
     size_t buffer_size),
    {
        const url_js = UTF8ToString(url);
        const method_js = UTF8ToString(method);
        const content_type_header_js = UTF8ToString(content_type_header);
        const authorization_header_js = UTF8ToString(authorization_header);

        try
        {
            const xhr = new XMLHttpRequest();
            xhr.open(method_js, url_js, false);
            xhr.responseType = "arraybuffer";
            if (content_type_header_js.length > 0)
            {
                xhr.setRequestHeader("Content-Type", content_type_header_js);
            }
            if (authorization_header_js.length > 0)
            {
                // Should this only be set if using https?  What about CORS via http?
                xhr.setRequestHeader("Authorization", authorization_header_js);
            }

            // Cache request info on JavaScript side so that it is available in subsequent calls
            // without having to pass it back and forth to/from C++.
            let request_index = 0;
            if (!Module["git2cpp_js_cache"])
            {
                Module["git2cpp_js_cache"] = {"next_index": request_index};
            }
            else
            {
                request_index = Module["git2cpp_js_cache"]["next_index"]++;
            }

            Module["git2cpp_js_cache"][request_index] = {xhr, result_buffer_pointer: 0, buffer_size};

            if (method_js == "GET")
            {
                xhr.send();
            }

            return request_index;
        }
        catch (err)
        {
            // Store error for later retrieval
            // clang-format off
            Module["git2cpp_js_error"] = { name: err.name ?? "", message : err.message ?? "" };
            // clang-format on
            console.error(err);
            return -1;
        }
    }
);

EM_JS(const char*, js_maybe_convert_url, (const char* url_str), {
    // Convert URL to use CORS proxy based on env vars GIT_CORS_PROXY and GIT_CORS_PROXY_TYPE.
    // If no conversion occurs, return the original unconverted URL as a new string.
    const url_js = UTF8ToString(url_str);
    const url = new URL(url_js);
    // clang-format off
    const env = Module["ENV"] ?? {};
    // clang-format on
    const GIT_CORS_PROXY = env["GIT_CORS_PROXY"];
    let ret = url_js;  // Default to returning original unconverted URL as new string.
    if (GIT_CORS_PROXY)
    {
        // clang-format off
        const GIT_CORS_PROXY_TYPE = env["GIT_CORS_PROXY_TYPE"] ?? "prefix";
        // clang-format on
        if (GIT_CORS_PROXY_TYPE == "prefix")
        {
            ret = GIT_CORS_PROXY;
            if (ret.at(-1) != '/')
            {
                ret += '/';
            }
            ret += url_js;
        }
        else if (GIT_CORS_PROXY_TYPE == "insert")
        {
            ret = url.protocol + "/" + GIT_CORS_PROXY;
            if (ret.at(-1) != '/')
            {
                ret += '/';
            }
            ret += url.host + url.pathname + url.search;
        }
        else
        {
            // clang-format off
            console.warn(`Invalid GIT_CORS_PROXY_TYPE of '${GIT_CORS_PROXY_TYPE}'`);
            // clang-format on
        }
    }
    return stringToNewUTF8(ret);
});

EM_JS(
    size_t,
    js_read,
    (int request_index,
     char* buffer,
     size_t buffer_size,
     int32_t* status,
     const char** status_text,
     const char** response_headers),
    {
        try
        {
            const cache = Module["git2cpp_js_cache"];
            const request = cache[request_index];
            const {xhr} = request;

            if (request.content)
            {
                xhr.send(request.content.buffer);
                request.content = null;
            }

            let bytes_read = 0;
            if (xhr.response && xhr.response.byteLength)
            {
                bytes_read = xhr.response.byteLength - request.result_buffer_pointer;
                if (bytes_read > buffer_size)
                {
                    bytes_read = buffer_size;
                }
            }

            // Caller must delete the returned status_text and response_headers.
            // clang-format off
            setValue(status, xhr.status, 'i32*');
            setValue(status_text, stringToNewUTF8(xhr.statusText ?? ""), 'i8**');
            setValue(response_headers, stringToNewUTF8(xhr.getAllResponseHeaders() ?? ""), 'i8**');
            // clang-format on

            if (bytes_read > 0)
            {
                const responseChunk = new Uint8Array(xhr.response, request.result_buffer_pointer, bytes_read);
                writeArrayToMemory(responseChunk, buffer);
                request.result_buffer_pointer += bytes_read;
            }
            return bytes_read
        }
        catch (err)
        {
            // Store error for later retrieval
            // clang-format off
            Module["git2cpp_js_error"] = { name: err.name ?? "", message : err.message ?? "" };
            // clang-format on
            console.error(err);
            return -1;
        }
    }
);

EM_JS(void, js_warning, (const char* msg), {
    const msg_js = UTF8ToString(msg);
    console.warning(msg_js);
});

EM_JS(size_t, js_write, (int request_index, const char* buffer, size_t buffer_size), {
    try
    {
        const cache = Module["git2cpp_js_cache"];
        const request = cache[request_index];
        // Note the slice(0) is important.
        const buffer_js = new Uint8Array(HEAPU8.buffer, buffer, buffer_size).slice(0);
        if (!request.content)
        {
            request.content = buffer_js;
        }
        else
        {
            const content = new Uint8Array(request.content.length + buffer_js.length);
            content.set(request.content);
            content.set(buffer_js, request.content.length);
            request.content = content;
        }
        return 0;
    }
    catch (err)
    {
        // Store error for later retrieval
        // clang-format off
        Module["git2cpp_js_error"] = { name: err.name ?? "", message : err.message ?? "" };
        // clang-format on
        console.error(err);
        return -1;
    }
});

// C wrapper functions that call JavaScript functions.

static std::string base64_encode(std::string_view str)
{
    // Use browser's base64 encoding.
    const char* encoded = js_base64_encode(str.data());
    std::string ret(encoded);
    delete encoded;  // Delete const char* allocated in JavaScript.
    return ret;
}

static void convert_js_to_git_error(wasm_http_stream* stream)
{
    // Convert error on JS side to git error.
    const char* error_str = js_get_error();
    if (std::string_view(error_str).starts_with("NetworkError:"))
    {
        git_error_set(
            GIT_ERROR_HTTP,
            "network error sending request to %s, see the browser console and/or network tab for more details",
            stream->m_unconverted_url.c_str()
        );
    }
    else
    {
        git_error_set(GIT_ERROR_HTTP, "%s", error_str);
    }
    delete error_str;  // Delete const char* allocated in JavaScript.
}

static int create_request(wasm_http_stream* stream, std::string_view content_header)
{
    stream->m_request_index = js_request(
        stream->get_full_url().c_str(),
        name_for_method(stream->m_service.m_method).c_str(),
        content_header.data(),
        stream->m_subtransport->m_authorization_header.c_str(),
        EMFORGE_BUFSIZE
    );
    return stream->m_request_index;
}

static void delete_request(wasm_http_stream* stream)
{
    if (stream->m_request_index != -1)
    {
        js_delete_request(stream->m_request_index);
        stream->m_request_index = -1;
    }
}

static int read(wasm_http_stream* stream, wasm_http_response& response, bool is_read_response)
{
    if (is_read_response)
    {
        // Response from a write.
        if (stream->m_request_index == -1)
        {
            git_error_set(
                GIT_ERROR_HTTP,
                "read_response called without pending request to %s",
                stream->m_unconverted_url.c_str()
            );
            return -1;
        }
    }
    else
    {
        if (stream->m_request_index != -1)
        {
            git_error_set(
                GIT_ERROR_HTTP,
                "read called with pending request to %s",
                stream->m_unconverted_url.c_str()
            );
            return -1;
        }

        if (create_request(stream, stream->m_service.m_response_type.c_str()) < 0)
        {
            convert_js_to_git_error(stream);
            return -1;
        }
    }

    const char* status_text = nullptr;
    const char* response_headers = nullptr;

    // Actual read.
    size_t bytes_read = js_read(
        stream->m_request_index,
        response.m_buffer,
        response.m_buffer_size,
        &response.m_status,
        &status_text,
        &response_headers
    );
    if (bytes_read < 0)
    {
        convert_js_to_git_error(stream);
        // Delete const char* allocated in JavaScript.
        delete status_text;
        delete response_headers;
        return -1;
    }

    response.m_status_text = status_text;
    delete status_text;  // Delete const char* allocated in JavaScript.

    // Split single string with response headers separated by \r\n into individual headers.
    auto lines = split_input_at_newlines(response_headers);
    for (const auto& line : lines)
    {
        auto pos = line.find(":");
        if (pos == std::string::npos)
        {
            // Skip invalid lines.  Should this be an error condition?
            continue;
        }
        response.add_header(line.substr(0, pos), line.substr(pos + 1));
    }
    delete response_headers;  // Delete const char* allocated in JavaScript.

    // If successful, check expected response content-type is correct.
    if (response.m_status == GIT_HTTP_STATUS_OK)
    {
        auto expected_response_type = stream->m_service.m_response_type;
        if (!expected_response_type.empty()
            && !response.has_header_matches("content-type", expected_response_type))
        {
            // Not sure this should be checked at all, as CORS proxy may be doing something
            // with it.
            git_error_set(
                GIT_ERROR_HTTP,
                "expected response content-type header '%s' to request %s",
                expected_response_type.c_str(),
                stream->m_unconverted_url.c_str()
            );
            return -1;
        }
    }

    *response.m_bytes_read = bytes_read;
    return 0;
}

static int write(wasm_http_stream* stream, const char* buffer, size_t buffer_size)
{
    if (stream->m_request_index == -1)
    {
        // If there is not already a request opened, do so now.
        if (create_request(stream, stream->m_service.m_request_type.c_str()) < 0)
        {
            convert_js_to_git_error(stream);
            return -1;
        }
    }

    int error = js_write(stream->m_request_index, buffer, buffer_size);
    if (error < 0)
    {
        convert_js_to_git_error(stream);
        return -1;
    }

    return 0;
}

// C credential functions.

static int create_credential(wasm_http_stream* stream, const wasm_http_response& response)
{
    wasm_http_subtransport* subtransport = stream->m_subtransport;

    // Delete old credential and authorization header.
    if (subtransport->m_credential != nullptr)
    {
        subtransport->m_credential->free(subtransport->m_credential);
        subtransport->m_credential = nullptr;
    }
    subtransport->m_authorization_header = "";

    // Check that response headers show support for 'www-authenticate: Basic'.
    if (!response.has_header_starts_with("www-authenticate", "Basic"))
    {
        git_error_set(
            GIT_ERROR_HTTP,
            "remote host for request %s does not support Basic authentication",
            stream->m_unconverted_url.c_str()
        );
        return -1;
    }

    // Get credentials from user via libgit2 registered callback.
    if (git_transport_smart_credentials(
            &subtransport->m_credential,
            subtransport->m_owner,
            nullptr,
            GIT_CREDENTIAL_USERPASS_PLAINTEXT
        )
        < 0)
    {
        // credentials_callback will have set git error.
        return -1;
    }

    if (subtransport->m_credential->credtype != GIT_CREDENTIAL_USERPASS_PLAINTEXT)
    {
        git_error_set(GIT_ERROR_HTTP, "Unexpected credential type");
        return -1;
    }

    // Create authorization header from username and password.
    // Cast is OK as checked above that credential is a GIT_CREDENTIAL_USERPASS_PLAINTEXT.
    auto userpass = reinterpret_cast<git_credential_userpass_plaintext*>(subtransport->m_credential);
    std::ostringstream buffer;
    buffer << userpass->username << ':' << userpass->password;
    subtransport->m_authorization_header = "Basic " + base64_encode(buffer.str());

    return 0;
}

// C wasm_http_stream functions.

wasm_http_stream::wasm_http_stream(wasm_http_subtransport* subtransport, http_service service)
    : m_subtransport(subtransport)
    , m_service(service)
    , m_request_index(-1)
{
}

bool wasm_http_stream::ensure_final_url(const std::string final_url)
{
    // Must be using a CORS proxy that has redirected, so store updated base URL to reuse.
    if (final_url.ends_with(m_service.m_url))
    {
        // Remove service URL from end of final URL to give new base URL.
        auto base_url = final_url.substr(0, final_url.size() - m_service.m_url.size());
        if (m_subtransport->m_base_url != base_url)
        {
            m_subtransport->m_base_url = base_url;
            return true;
        }
    }
    else
    {
        std::string msg = "Unexpected x-final-url: " + final_url;
        js_warning(msg.c_str());
    }
    return false;
}

bool wasm_http_stream::ensure_https()
{
    const std::string http = "http:";
    if (m_subtransport->m_base_url.starts_with(http))
    {
        m_subtransport->m_base_url.replace(0, http.size(), "https:");
        return true;
    }
    return false;
}

std::string wasm_http_stream::get_full_url()
{
    // Base URL never ends with a slash, service URL always begins with a slash.
    m_unconverted_url = m_subtransport->m_base_url + m_service.m_url;

    const char* converted_url = js_maybe_convert_url(m_unconverted_url.c_str());
    std::string ret = converted_url;
    delete converted_url;  // Delete const char* allocated in JavaScript.
    return ret;
}

void wasm_http_stream_free(git_smart_subtransport_stream* s)
{
    wasm_http_stream* stream = reinterpret_cast<wasm_http_stream*>(s);
    delete_request(stream);
    delete stream;
}

int wasm_http_stream_read(git_smart_subtransport_stream* s, char* buffer, size_t buffer_size, size_t* bytes_read)
{
    wasm_http_stream* stream = reinterpret_cast<wasm_http_stream*>(s);
    wasm_http_response response(buffer, buffer_size, bytes_read);

    bool send = true;
    while (send)
    {
        if (read(stream, response, false) < 0)
        {
            return -1;  // git error already set.
        }
        send = false;

        auto final_url_header = response.get_header("x-final-url");
        if (final_url_header.has_value() && stream->ensure_final_url(final_url_header.value())
            && response.m_status != GIT_HTTP_STATUS_OK)
        {
            // Resend only if status not OK, if OK next request will use updated URL.
            send = true;
        }

        if (response.has_header("strict-transport-security") && stream->ensure_https()
            && response.m_status != GIT_HTTP_STATUS_OK)
        {
            // Resend only if status not OK, if OK next request will use https not http.
            send = true;
        }

        if (response.m_status == GIT_HTTP_STATUS_UNAUTHORIZED)
        {
            // Request and create new credentials.
            if (create_credential(stream, response) < 0)
            {
                return -1;  // git error already set.
            }
            send = true;  // Resend will use updated credentials.
        }

        if (send)
        {
            delete_request(stream);
            response.clear();
        }
    }

    if (response.m_status != GIT_HTTP_STATUS_OK)
    {
        response.set_git_error(stream->m_unconverted_url);
        return -1;
    }

    return 0;
}

int wasm_http_stream_read_response(git_smart_subtransport_stream* s, char* buffer, size_t buffer_size, size_t* bytes_read)
{
    wasm_http_stream* stream = reinterpret_cast<wasm_http_stream*>(s);

    wasm_http_response response(buffer, buffer_size, bytes_read);
    int error = read(stream, response, true);

    // May need similar handling of response status and headers as occurs in read() above, but so
    // far this has not been necessary.

    if (error == 0 && response.m_status != GIT_HTTP_STATUS_OK)
    {
        response.set_git_error(stream->m_unconverted_url);
        error = -1;
    }

    return error;
}

int wasm_http_stream_write(git_smart_subtransport_stream* s, const char* buffer, size_t buffer_size)
{
    wasm_http_stream* stream = reinterpret_cast<wasm_http_stream*>(s);
    return write(stream, buffer, buffer_size);
}

#endif  // EMSCRIPTEN
