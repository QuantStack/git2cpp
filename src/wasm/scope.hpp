#pragma once

#include "../utils/common.hpp"

// Scope object to enable/disable browser-based wasm http transport scheme for libgit2.
// This is a no-op unless EMSCRIPTEN is defined.
class wasm_http_transport_scope : noncopyable_nonmovable
{
public:

    wasm_http_transport_scope();

    ~wasm_http_transport_scope();
};
