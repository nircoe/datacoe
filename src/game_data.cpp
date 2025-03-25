#include "game_data.hpp"
#include <iostream>
#include <stdexcept>

namespace datacoe
{
    GameData::GameData(const std::string &nickname, const int highscore) : m_nickname(nickname), m_highscore(highscore) {}

    void GameData::setNickName(const std::string &nickname) { m_nickname = nickname; }

    void GameData::setHighScore(int highscore) { m_highscore = highscore; }

    const std::string &GameData::getNickName() const { return m_nickname; }

    int GameData::getHighscore() const { return m_highscore; }

    json GameData::toJson() const
    {
        json j;
        j["nickname"] = m_nickname;
        j["highscore"] = m_highscore;
        return j;
    }

    GameData GameData::fromJson(const json &j)
    {
        std::string nickname;
        int highscore;
        if (j.contains("nickname") && j["nickname"].is_string())
        {
            try
            {
                nickname = j["nickname"].get<std::string>();
            }
            catch (const json::exception &)
            {
                throw;
            }
        }
        else
            throw std::runtime_error("'nickname' key is missing or invalid in the JSON object we are trying to load.");

        if (j.contains("highscore") && j["highscore"].is_number_integer())
        {
            try
            {
                highscore = j["highscore"].get<int>();
            }
            catch (const json::exception &)
            {
                throw;
            }
        }
        else
            throw std::runtime_error("'highscore' key is missing or invalid in the JSON object we are trying to load.");

        return GameData(nickname, highscore);
    }
} // namespace datacoe