#ifdef EMSCRIPTEN

#    include "transport.hpp"

#    include "subtransport.hpp"

// git_transport_cb
int create_wasm_http_transport(git_transport** out, git_remote* owner, void* param)
{
    git_smart_subtransport_definition definition;
    definition.callback = create_wasm_http_subtransport;
    definition.rpc = true;
    definition.param = param;
    return git_transport_smart(out, owner, &definition);
}

#endif  // EMSCRIPTEN
