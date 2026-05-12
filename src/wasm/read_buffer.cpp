#ifdef EMSCRIPTEN

#    include "read_buffer.hpp"

read_buffer_t::read_buffer_t(char* buffer, size_t buffer_size, size_t* bytes_read)
    : m_buffer(buffer)
    , m_buffer_size(buffer_size)
    , m_bytes_read(bytes_read)
{
}

#endif  // EMSCRIPTEN
