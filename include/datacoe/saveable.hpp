#pragma once

#include <nlohmann/json.hpp>
#include <concepts>

namespace datacoe
{
    // Put NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT right after the struct, at namespace scope and in the
    // same namespace as the struct, so ADL can find it.
    template <typename T>
    concept saveable = std::semiregular<T> && requires(const T &value, const nlohmann::json &j)
    {
        nlohmann::json(value);
        { j.get<T>() } -> std::same_as<T>;
    };
} // namespace datacoe
