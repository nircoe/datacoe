#pragma once

#include <string>
#include <array>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace datacoe
{
    class GameData
    {
        std::string m_nickname;
        std::array<int, 4> m_highscores;

    public:
        GameData(const std::string& nickname = "", const std::array<int, 4>& highscores = {0});

        void setNickname(const std::string& nickname);
        void setHighscores(const std::array<int, 4>& highscores);

        const std::string& getNickname() const;
        const std::array<int, 4>& getHighscores() const;

        json toJson() const;

        static GameData fromJson(const json& j);
    };
} // namespace datacoe