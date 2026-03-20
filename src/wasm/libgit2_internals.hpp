#pragma once

#ifdef EMSCRIPTEN

#    include <optional>
#    include <string>

#    include <git2/sys/errors.h>
#    include <git2/sys/transport.h>

// Libgit2 internals that we want to use so they are reproduced here in some form.

// asserts

#    define GIT_ASSERT(expr) GIT_ASSERT_WITH_RETVAL(expr, -1)

#    define GIT_ASSERT_ARG(expr) GIT_ASSERT_ARG_WITH_RETVAL(expr, -1)

#    define GIT_ASSERT_WITH_RETVAL(expr, fail) \
        GIT_ASSERT__WITH_RETVAL(expr, 0, "unrecoverable internal error", fail)

#    define GIT_ASSERT_ARG_WITH_RETVAL(expr, fail) GIT_ASSERT__WITH_RETVAL(expr, 0, "invalid argument", fail)

#    define GIT_ASSERT__WITH_RETVAL(expr, code, msg, fail)   \
        do                                                   \
        {                                                    \
            if (!(expr))                                     \
            {                                                \
                git_error_set(code, "%s: '%s'", msg, #expr); \
                return fail;                                 \
            }                                                \
        } while (0)

// http status code, method and service.

#    define GIT_HTTP_STATUS_CONTINUE 100
#    define GIT_HTTP_STATUS_OK 200
#    define GIT_HTTP_MOVED_PERMANENTLY 301
#    define GIT_HTTP_FOUND 302
#    define GIT_HTTP_SEE_OTHER 303
#    define GIT_HTTP_TEMPORARY_REDIRECT 307
#    define GIT_HTTP_PERMANENT_REDIRECT 308
#    define GIT_HTTP_STATUS_UNAUTHORIZED 401
#    define GIT_HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED 407

typedef enum
{
    GIT_HTTP_METHOD_GET,
    GIT_HTTP_METHOD_POST,
    GIT_HTTP_METHOD_CONNECT
} git_http_method;

typedef struct
{
    git_http_method m_method;
    std::string m_url;
    std::string m_request_type;
    std::string m_response_type;
    unsigned int m_initial : 1, m_chunked : 1;
} http_service;

std::string name_for_method(git_http_method method);

std::optional<http_service> select_service(git_smart_service_t action);

#endif  // EMSCRIPTEN
