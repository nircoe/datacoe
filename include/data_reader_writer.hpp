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
        static bool isFileEncrypted(const std::string &filename);
        static bool writeData(const GameData &gamedata, const std::string &filename, bool encryption = true);
        static std::optional<GameData> readData(const std::string &filename, bool decryption = true);
    };
} // namespace datacoe