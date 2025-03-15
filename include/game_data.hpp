#pragma once

#include <string>
#include <array>
#include "json.hpp"

using json = nlohmann::json;

namespace DataManagement
{
    // Users should modify this file to match their own game data
    class GameData 
    {
        // game data examples
        std::string m_nickname;
        int m_highscore;

    public:
        GameData(const std::string& nickname = "", const int highscore = 0);

        void setNickName(const std::string& nickname);
        void setHighScore(int highscore);

        const std::string& getNickName() const;
        const int getHighscore() const;

        json toJson() const;

        static GameData fromJson(const json& j);    
    };
} // namespace DataManagement