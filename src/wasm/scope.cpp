#include "scope.hpp"

#ifdef __EMSCRIPTEN__
#    include "transport.hpp"
#endif

wasm_http_transport_scope::wasm_http_transport_scope()
{
#ifdef __EMSCRIPTEN__
    git_transport_register("http", create_wasm_http_transport, nullptr);
    git_transport_register("https", create_wasm_http_transport, nullptr);
#endif
}

wasm_http_transport_scope::~wasm_http_transport_scope()
{
#ifdef __EMSCRIPTEN__
    git_transport_unregister("http");
    git_transport_unregister("https");
#endif
}
