#include "gtest/gtest.h"
#include "data_manager.hpp"
#include "data_reader_writer.hpp"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

namespace datacoe
{
    class IntegrationTest : public ::testing::Test
    {
    protected:
        std::string m_testFilename;
        
        void SetUp() override
        {
            m_testFilename = "test_integration.json";
            // Clean up any leftover files
            try
            {
                if (std::filesystem::exists(m_testFilename))
                {
                    std::filesystem::remove(m_testFilename);
                }
            }
            catch (const std::filesystem::filesystem_error &)
            {
                // Ignore if file doesn't exist
            }
        }

        void TearDown() override
        {
            // Give file handles time to close
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Try to clean up the test file
            for (int i = 0; i < 5; i++)
            {
                try
                {
                    if (std::filesystem::exists(m_testFilename))
                    {
                        std::filesystem::remove(m_testFilename);
                    }
                    break;
                }
                catch (const std::filesystem::filesystem_error &e)
                {
                    std::cerr << "TearDown() Error: " << e.what() << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }
    };

    TEST_F(IntegrationTest, FullLifecycle)
    {
        try
        {
            // 1. Create game data
            GameData originalData("IntegrationTest", 1000);

            // 2. Write directly with DataReaderWriter
            ASSERT_TRUE(DataReaderWriter::writeData(originalData, m_testFilename));

            // 3. Load with DataManager
            DataManager dm;
            dm.init(m_testFilename);

            // 4. Verify data loaded correctly
            const GameData &loadedData = dm.getGameData();
            ASSERT_EQ(loadedData.getNickName(), "IntegrationTest");
            ASSERT_EQ(loadedData.getHighscore(), 1000);

            // 5. Modify and save with DataManager
            dm.setHighScore(2000);
            ASSERT_TRUE(dm.saveGame());

            // 6. Read directly with DataReaderWriter
            std::optional<GameData> readData = DataReaderWriter::readData(m_testFilename);
            ASSERT_TRUE(readData.has_value());
            ASSERT_EQ(readData.value().getNickName(), "IntegrationTest");
            ASSERT_EQ(readData.value().getHighscore(), 2000);
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

    TEST_F(IntegrationTest, MultipleInstances)
    {
        try
        {
            // Create and use multiple DataManager instances with the same file
            DataManager dm1;
            dm1.init(m_testFilename);
            dm1.setNickName("Player1");
            dm1.setHighScore(100);
            ASSERT_TRUE(dm1.saveGame());

            // Create a second instance and load the data
            DataManager dm2;
            dm2.init(m_testFilename);
            ASSERT_EQ(dm2.getGameData().getNickName(), "Player1");
            ASSERT_EQ(dm2.getGameData().getHighscore(), 100);

            // Modify with the second instance
            dm2.setHighScore(200);
            ASSERT_TRUE(dm2.saveGame());

            // Create a third instance and check data
            DataManager dm3;
            dm3.init(m_testFilename);
            ASSERT_EQ(dm3.getGameData().getNickName(), "Player1");
            ASSERT_EQ(dm3.getGameData().getHighscore(), 200);

            // Original instance should still have old data in memory
            ASSERT_EQ(dm1.getGameData().getHighscore(), 100);

            // After reloading, it should see the new data
            dm1.loadGame();
            ASSERT_EQ(dm1.getGameData().getHighscore(), 200);
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

    TEST_F(IntegrationTest, DataCorruption)
    {
        try
        {
            // Setup initial valid data
            DataManager dm1;
            dm1.init(m_testFilename);
            dm1.setNickName("ValidData");
            dm1.setHighScore(100);
            ASSERT_TRUE(dm1.saveGame());

            // Corrupt the file
            {
                std::ofstream file(m_testFilename, std::ios::trunc);
                file << "This is corrupted data that can't be decrypted";
                file.close();
            }

            // Try to load corrupted data
            DataManager dm2;
            dm2.init(m_testFilename);

            // Should initialize with default empty values
            ASSERT_EQ(dm2.getGameData().getNickName(), "");
            ASSERT_EQ(dm2.getGameData().getHighscore(), 0);

            // Save new data
            dm2.setNickName("RecoveredData");
            dm2.setHighScore(300);
            ASSERT_TRUE(dm2.saveGame());

            // Verify the new data was saved correctly
            DataManager dm3;
            dm3.init(m_testFilename);
            ASSERT_EQ(dm3.getGameData().getNickName(), "RecoveredData");
            ASSERT_EQ(dm3.getGameData().getHighscore(), 300);
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

} // namespace datacoe