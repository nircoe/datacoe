#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace datacoe
{
    // Users should modify this file to match their own game data
    class GameData
    {
        // game data examples
        std::string m_nickname;
        int m_highscore;

    public:
        GameData(const std::string &nickname = "", const int highscore = 0);

        void setNickname(const std::string &nickname);
        void setHighscore(int highscore);

        const std::string &getNickname() const;
        int getHighscore() const;

        json toJson() const;

        static GameData fromJson(const json &j);
    };
} // namespace datacoe