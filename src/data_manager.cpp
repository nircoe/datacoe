#include "data_manager.hpp"

namespace DataManagement
{
    bool DataManager::saveGame()
    {
        if(m_gamedata.getNickName().empty())
            return true; // no need to save (guest mode)

        return DataReaderWriter::writeData(m_gamedata, m_filename);
    }

    bool DataManager::loadGame()
    {
        std::optional<GameData> loadedGameData = DataReaderWriter::readData(m_filename);
        bool readDataSucceed = loadedGameData.has_value();
        if(readDataSucceed)
            m_gamedata = loadedGameData.value();
        return readDataSucceed;
    }

    void DataManager::init(const std::string filename)
    {
        m_filename = filename;
        if(!loadGame())
        {
            // can't load, needs to ask the user for a nickname and create new GameData
            newGame();
        }
    }

    void DataManager::newGame()
    {
        m_gamedata = GameData();
    }

    void DataManager::setNickName(std::string nickname)
    {
        m_gamedata.setNickName(nickname);
    }

    void DataManager::setHighScore(int highscore)
    {
        m_gamedata.setHighScore(highscore);
    }

    const GameData& DataManager::getGameData() const
    {
        return m_gamedata;
    }
} // namespace DataManagement