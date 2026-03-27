#pragma once

#ifdef EMSCRIPTEN

// Get wasm http request timeout in milliseconds from environment variable or default value.
unsigned long get_request_timeout_ms();

#endif  // EMSCRIPTEN
