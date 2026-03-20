#pragma once

#ifdef EMSCRIPTEN

#    include <git2/sys/transport.h>

// Callback of type git_transport_cb that is registered with libgit2 and is called to handle
// http(s) transport.
int create_wasm_http_transport(git_transport** out, git_remote* owner, void* param);

#endif  // EMSCRIPTEN
