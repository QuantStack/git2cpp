#ifdef EMSCRIPTEN

#    include "libgit2_internals.hpp"

// http method and service.

std::string name_for_method(git_http_method method)
{
    switch (method)
    {
        case GIT_HTTP_METHOD_GET:
            return "GET";
        case GIT_HTTP_METHOD_POST:
            return "POST";
        case GIT_HTTP_METHOD_CONNECT:
            return "CONNECT";
    }
    return "";
}

std::optional<http_service> select_service(git_smart_service_t action)
{
    switch (action)
    {
        case GIT_SERVICE_UPLOADPACK_LS:
            return http_service{
                GIT_HTTP_METHOD_GET,
                "/info/refs?service=git-upload-pack",
                nullptr,
                "application/x-git-upload-pack-advertisement",
                1,
                0
            };
        case GIT_SERVICE_UPLOADPACK:
            return http_service{
                GIT_HTTP_METHOD_POST,
                "/git-upload-pack",
                "application/x-git-upload-pack-request",
                "application/x-git-upload-pack-result",
                0,
                0
            };
        case GIT_SERVICE_RECEIVEPACK_LS:
            return http_service{
                GIT_HTTP_METHOD_GET,
                "/info/refs?service=git-receive-pack",
                nullptr,
                "application/x-git-receive-pack-advertisement",
                1,
                0
            };
        case GIT_SERVICE_RECEIVEPACK:
            return http_service{
                GIT_HTTP_METHOD_POST,
                "/git-receive-pack",
                "application/x-git-receive-pack-request",
                "application/x-git-receive-pack-result",
                0,
                1
            };
    }

    return std::nullopt;
}

#endif  // EMSCRIPTEN
