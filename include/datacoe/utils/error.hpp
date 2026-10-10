#pragma once

#include <string>

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
} // namespace datacoe
