#include "datacoe/data_manager.hpp"
#include "datacoe/data_reader_writer.hpp"
#include <optional>

namespace datacoe
{
    bool DataManager::init(const std::string filename, bool encrypt)
    {
        m_filename = filename;
        m_encrypt = encrypt;

        return loadGame(); // if succeed, returns true, if not, returns false and GameManager will start new game
    }

    bool DataManager::saveGame()
    {
        if (m_gamedata.getNickname().empty())
            return true; // no need to save (guest mode)

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

    void DataManager::newGame(const std::string &nickname)
    {
        m_gamedata = GameData(nickname);
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