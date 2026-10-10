#pragma once

#include <datacoe/saveable.hpp>
#include <datacoe/utils/error.hpp>
#include <datacoe/utils/io.hpp>
#include <nlohmann/json.hpp>
#include <exception>
#include <expected>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <utility>

namespace datacoe
{
    // The body sits in an if constexpr so a type that isn't saveable only shows the static_assert message,
    // without the template error wall from nlohmann. The else branch is never reached.
    template <typename T>
    [[nodiscard]] std::expected<void, error> save(const std::filesystem::path &path, const T &data,
                                                  bool encrypt = false)
    {
        static_assert(saveable<T>,
                      "datacoe::save<T>: T is not saveable. It needs a default constructor, copy support and "
                      "to_json/from_json, add NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(T, members...) "
                      "right after the struct");

        if constexpr (saveable<T>)
        {
            constexpr std::string_view NAME = "datacoe::save()";

            std::string text;
            try
            {
                const nlohmann::json j = data;
                text = j.dump();
            }
            catch (const std::exception &e)
            {
                return std::unexpected(error{error_code::serialization_failure, std::format("{}: {}", NAME, e.what())});
            }

            if (encrypt)
            {
                auto encrypted = internal::encrypt(text);
                if (!encrypted)
                    return std::unexpected(internal::with_prefix(NAME, encrypted.error()));
                text = std::move(*encrypted);
            }

            auto written = internal::write_file(path, text);
            if (!written)
                return std::unexpected(internal::with_prefix(NAME, written.error()));
            return {};
        }
        else
        {
            return std::unexpected(error{});
        }
    }

    // The body sits in an if constexpr for the same reason as in save.
    template <typename T>
    [[nodiscard]] std::expected<T, error> load(const std::filesystem::path &path)
    {
        static_assert(saveable<T>,
                      "datacoe::load<T>: T is not saveable. It needs a default constructor, copy support and "
                      "to_json/from_json, add NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(T, members...) "
                      "right after the struct");

        if constexpr (saveable<T>)
        {
            constexpr std::string_view NAME = "datacoe::load()";

            // The file is opened twice. A writer replacing it between the two opens could pair the old prefix
            // check with the new contents. Concurrent writers aren't supported, same as for save.
            const auto is_encrypted = internal::is_file_encrypted(path);
            if (!is_encrypted)
                return std::unexpected(internal::with_prefix(NAME, is_encrypted.error()));

            auto contents = internal::read_file(path);
            if (!contents)
                return std::unexpected(internal::with_prefix(NAME, contents.error()));

            if (*is_encrypted)
            {
                auto plain = internal::decrypt(*contents);
                if (!plain)
                    return std::unexpected(internal::with_prefix(NAME, plain.error()));
                contents = std::move(plain);
            }

            nlohmann::json j;
            try
            {
                j = nlohmann::json::parse(*contents);
            }
            catch (const std::exception &e)
            {
                return std::unexpected(error{error_code::parse_failure, std::format("{}: {}", NAME, e.what())});
            }

            try
            {
                return j.get<T>();
            }
            catch (const std::exception &e)
            {
                return std::unexpected(
                    error{error_code::deserialization_failure, std::format("{}: {}", NAME, e.what())});
            }
        }
        else
        {
            return std::unexpected(error{});
        }
    }
} // namespace datacoe
