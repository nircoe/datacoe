#include "datacoe/game_data.hpp"
#include <stdexcept>

namespace datacoe
{
    GameData::GameData(const std::string &nickname, const std::array<std::size_t, 4> &highscores) : m_nickname(nickname), m_highscores(highscores) {}

    void GameData::setNickname(const std::string &nickname) { m_nickname = nickname; }

    void GameData::setHighscores(const std::array<std::size_t, 4> &highscores) { m_highscores = highscores; }

    const std::string &GameData::getNickname() const { return m_nickname; }

    const std::array<std::size_t, 4> &GameData::getHighscores() const { return m_highscores; }

    json GameData::toJson() const
    {
        json j;
        j["nickname"] = m_nickname;
        j["highscores"] = m_highscores;
        return j;
    }

    GameData GameData::fromJson(const json &j)
    {
        std::string nickname;
        std::array<std::size_t, 4> highscores;
        if (j.contains("nickname") && j["nickname"].is_string())
        {
            try { nickname = j["nickname"].get<std::string>(); }
            catch (const json::exception &) { throw; }
        }
        else
            throw std::runtime_error("'nickname' key is missing or invalid in the JSON object we are trying to load.");

        if (j.contains("highscores") && j["highscores"].is_array() && j["highscores"].size() == 4)
        {
            try { highscores = j["highscores"].get<std::array<std::size_t, 4>>(); }
            catch (const json::exception &) { throw; }
        }
        else
            throw std::runtime_error("'highscores' key is missing or invalid in the JSON object we are trying to load.");

        return GameData(nickname, highscores);
    }
} // namespace datacoe