#pragma once

#ifdef EMSCRIPTEN

// Get wasm http request timeout in milliseconds from environment variable or default value.
unsigned long get_request_timeout_ms();

// Return true if OK to continue with download, false otherwise.
// May or may not prompt the user for input depending on the size of the download.
bool maybe_prompt_to_download(double bytes);

#endif  // EMSCRIPTEN
