#include "datacoe/data_manager.hpp"

namespace datacoe
{
    bool DataManager::init(const std::string filename, bool encrypt)
    {
        m_filename = filename;
        m_encrypt = encrypt;

        if (!loadGame())
        {
            // can't load, needs to ask the user for a nickname and create new GameData
            // or change for you own game logic
            newGame();
            return false;
        }
        return true;
    }

    bool DataManager::saveGame()
    {
        if (m_gamedata.getNickname().empty())
            return true; // no need to save (guest mode), modify for you own game logic

        bool result = DataReaderWriter::writeData(m_gamedata, m_filename, m_encrypt);
        if (result)
            m_fileEncrypted = m_encrypt;

        return result;
    }

    bool DataManager::loadGame()
    {
        m_fileEncrypted = DataReaderWriter::isFileEncrypted(m_filename);

        std::optional<GameData> loadedGamedata = DataReaderWriter::readData(m_filename, m_encrypt);
        bool readDataSucceed = loadedGamedata.has_value();
        if (readDataSucceed)
            m_gamedata = loadedGamedata.value();
        return readDataSucceed;
    }

    void DataManager::newGame()
    {
        m_gamedata = GameData();
    }

    void DataManager::setGamedata(const GameData &gamedata)
    {
        m_gamedata = gamedata;
    }

    const GameData &DataManager::getGamedata() const
    {
        return m_gamedata;
    }

    bool DataManager::isEncrypted() const
    {
        return m_fileEncrypted;
    }

    void DataManager::setEncryption(bool encrypt)
    {
        m_encrypt = encrypt;
    }
} // namespace datacoe