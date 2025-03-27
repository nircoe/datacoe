#pragma once

#include <string>
#include <optional>
#include "game_data.hpp"
#include "data_reader_writer.hpp"

namespace datacoe
{
    class DataManager
    {
        std::string m_filename;
        GameData m_gamedata;
        bool m_encrypt = true;             // Whether to use encryption
        bool m_fileEncrypted = false;       // Whether the file is currently encrypted

    public:
        // Users should add or modify constructors and destructor as needed
        DataManager() = default;
        ~DataManager() = default;

        // Users should modify those methods to match their own game
        bool saveGame();
        bool loadGame();

        // Users should modify the initialization to match their own game
        void init(const std::string filename, bool encrypt = true);

        // Users should modify this method to match their own game
        void newGame();

        // GameData specific methods, Users should modify to match their own game
        void setNickName(std::string nickname);
        void setHighScore(int highscore);

        const GameData &getGameData() const;

        // Encryption related methods
        bool isEncrypted() const;
        void setEncryption(bool encrypt);
    };
} // namespace datacoe