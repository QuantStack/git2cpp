#pragma once

#ifdef EMSCRIPTEN

#    include <stddef.h>

// Buffer used when reading remote data is created by libgit2 and passed to transport layer to use.
// We only fill the buffer and otherwise return it unmodified.
struct read_buffer_t
{
    read_buffer_t(char* buffer, size_t buffer_size, size_t* bytes_read);

    char* m_buffer;
    size_t m_buffer_size;
    size_t* m_bytes_read;
};

#endif  // EMSCRIPTEN
