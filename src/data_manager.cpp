#include "data_manager.hpp"

namespace datacoe
{
    bool DataManager::saveGame()
    {
        if (m_gamedata.getNickName().empty())
            return true; // no need to save (guest mode)

        bool result = DataReaderWriter::writeData(m_gamedata, m_filename, m_encrypt);
        if (result)
            m_fileEncrypted = m_encrypt;

        return result;
    }

    bool DataManager::loadGame()
    {
        m_fileEncrypted = DataReaderWriter::isFileEncrypted(m_filename);

        std::optional<GameData> loadedGameData = DataReaderWriter::readData(m_filename, m_encrypt);
        bool readDataSucceed = loadedGameData.has_value();
        if (readDataSucceed)
            m_gamedata = loadedGameData.value();
        return readDataSucceed;
    }

    void DataManager::init(const std::string filename, bool encrypt)
    {
        m_filename = filename;
        m_encrypt = encrypt;

        if (!loadGame())
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

    const GameData &DataManager::getGameData() const
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