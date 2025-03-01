#pragma once

#include <string>
#include <optional>
#include "game_data.hpp"

namespace datacoe
{
    // No need to modify
    class DataReaderWriter
    {
        static std::string encrypt(const std::string &data);
        static std::string decrypt(const std::string &encodedData);

    public:
        static bool writeData(const GameData &gamedata, const std::string &filename);
        static std::optional<GameData> readData(const std::string &filename);
    };
} // namespace datacoe