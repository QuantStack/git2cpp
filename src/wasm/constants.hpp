#pragma once

#include <string_view>

// Constants used in wasm transport layer.
// Exposed to non-emscripten builds so that they can be used in help.

// Environment variable for http transport timeout.
// Must be a positive number as a timeout of 0 will block forever.
inline constexpr std::string_view WASM_HTTP_TRANSPORT_TIMEOUT_NAME = "GIT_HTTP_TIMEOUT";
inline constexpr unsigned int WASM_HTTP_TRANSPORT_TIMEOUT_DEFAULT = 10;
