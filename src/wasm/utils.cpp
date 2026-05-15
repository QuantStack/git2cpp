#ifdef EMSCRIPTEN

#    include "utils.hpp"

#    include <algorithm>
#    include <cmath>
#    include <iomanip>
#    include <sstream>

#    include <termcolor/termcolor.hpp>

#    include "../utils/input_output.hpp"
#    include "constants.hpp"

unsigned long get_request_timeout_ms()
{
    double timeout_seconds = WASM_HTTP_TRANSPORT_TIMEOUT_DEFAULT_S;
    auto env_var = std::getenv(WASM_HTTP_TRANSPORT_TIMEOUT_NAME.data());
    if (env_var != nullptr)
    {
        try
        {
            auto value = std::stod(env_var);
            if (value < 1e-3)  // Must be at least 1 ms.
            {
                throw std::runtime_error("");  // Caught below.
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
                      << WASM_HTTP_TRANSPORT_TIMEOUT_DEFAULT_S << " seconds instead." << termcolor::reset
                      << std::endl;
        }
    }
    return 1000 * timeout_seconds;
}

std::string human_readable_size(double bytes)
{
    static constexpr const char* units[] = {"B", "kB", "MB", "GB", "TB"};
    int nunits = sizeof(units) / sizeof(units[0]);
    int index = std::trunc(std::log10(bytes) / 3);
    index = std::clamp(index, 0, nunits - 1, std::less());

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << (bytes / std::pow(1000.0, index)) << ' ' << units[index];
    return oss.str();
}

bool maybe_prompt_to_download(double bytes)
{
    if (bytes <= 0.0)
    {
        return true;
    }

    // Limit
    double bytes_limit = 1e7;  // 10 MB
    if (bytes > bytes_limit)
    {
        if (!termcolor::_internal::is_atty(std::cout))
        {
            // If not a tty cannot prompt user so prevent the download.
            return false;
        }

        std::string prompt = "Download size is " + human_readable_size(bytes) + ", continue [y/N]? ";
        return prompt_yes_or_no(prompt, false);
    }

    return true;
}

#endif  // EMSCRIPTEN
