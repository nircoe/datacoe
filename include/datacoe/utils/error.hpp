#pragma once

#include <format>
#include <string>
#include <string_view>

namespace datacoe
{
    enum class error_code
    {
        decryption_failure,
        deserialization_failure,
        encryption_failure,
        file_not_found,
        file_read_failure,
        file_write_failure,
        invalid_argument,
        parse_failure,
        serialization_failure
    };

    struct error
    {
        error_code code;
        std::string message;
    };

    namespace internal
    {
        [[nodiscard]] inline error with_prefix(std::string_view function, error e)
        {
            e.message = std::format("{}: {}", function, e.message);
            return e;
        }
    } // namespace internal
} // namespace datacoe
