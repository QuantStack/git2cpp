#ifdef EMSCRIPTEN

#    include "utils.hpp"

#    include <termcolor/termcolor.hpp>

#    include "constants.hpp"

unsigned long get_request_timeout_ms()
{
    double timeout_seconds = WASM_HTTP_TRANSPORT_TIMEOUT_DEFAULT;
    auto env_var = std::getenv(WASM_HTTP_TRANSPORT_TIMEOUT_NAME.data());
    if (env_var != nullptr)
    {
        try
        {
            auto value = std::stod(env_var);
            if (value <= 0)
            {
                throw std::runtime_error("negative or zero");
            }
            timeout_seconds = value;
        }
        catch (std::exception& e)
        {
            // Catch failures from (1) stod and (2) timeout <= 0.
            // Print warning and use default value.
            std::cout << termcolor::yellow << "Warning: environment variable "
                      << WASM_HTTP_TRANSPORT_TIMEOUT_NAME
                      << " must be a positive number of seconds, using default value of "
                      << WASM_HTTP_TRANSPORT_TIMEOUT_DEFAULT << " seconds instead." << termcolor::reset
                      << std::endl;
        }
    }
    return 1000 * timeout_seconds;
}

#endif  // EMSCRIPTEN
