#pragma once

// Not public API. Included by the save and load templates, can change without notice.

#include <datacoe/utils/error.hpp>
#include <expected>
#include <filesystem>
#include <string>

namespace datacoe
{
    namespace internal
    {
        [[nodiscard]] std::expected<bool, error> is_file_encrypted(const std::filesystem::path &path);
        [[nodiscard]] std::expected<std::string, error> encrypt(const std::string &plaintext);
        [[nodiscard]] std::expected<std::string, error> decrypt(const std::string &file_contents);
        [[nodiscard]] std::expected<std::string, error> read_file(const std::filesystem::path &path);
        [[nodiscard]] std::expected<void, error> write_file(const std::filesystem::path &path,
                                                            const std::string &contents);
    } // namespace internal
} // namespace datacoe
