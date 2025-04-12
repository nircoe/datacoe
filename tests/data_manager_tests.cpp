#include "gtest/gtest.h"
#include <datacoe/data_manager.hpp>
#include <datacoe/data_reader_writer.hpp>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <iostream>

namespace datacoe
{
    class DataManagerTest : public ::testing::Test
    {
    protected:
        std::string m_testFilename;

        void SetUp() override
        {
            m_testFilename = "test_data_manager.json";
            // Clean up any leftover files from previous tests
            try
            {
                if (std::filesystem::exists(m_testFilename))
                {
                    std::filesystem::remove(m_testFilename);
                }
            }
            catch (const std::filesystem::filesystem_error &)
            {
                // Ignore errors if file doesn't exist
            }
        }

        void TearDown() override
        {
            // Give a small delay to ensure file handles are released
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Try to clean up the test file
            for (std::size_t i = 0; i < 5; i++)
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

    TEST_F(DataManagerTest, SaveAndLoadGame)
    {
        try
        {
            // Create GameData and set in DataManager
            DataManager dm;
            dm.init(m_testFilename);

            // Create and set GameData
            GameData data;
            data.setNickname("TestUser");
            std::array<std::size_t, 4> scores = {100, 200, 300, 400};
            data.setHighscores(scores);
            dm.setGamedata(data);

            ASSERT_TRUE(dm.saveGame()) << "Failed to save game";
            ASSERT_TRUE(std::filesystem::exists(m_testFilename)) << "Save file not created";

            // Create a new DataManager to load the saved data
            DataManager dm2;
            dm2.init(m_testFilename);

            const GameData &loadedData = dm2.getGamedata();

            ASSERT_EQ(loadedData.getNickname(), "TestUser");
            ASSERT_EQ(loadedData.getHighscores(), scores);
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

    TEST_F(DataManagerTest, SaveAndUpdateGame)
    {
        try
        {
            DataManager dm;
            dm.init(m_testFilename);

            // Set initial data and save
            GameData data;
            data.setNickname("TestUser");
            std::array<std::size_t, 4> initialScores = {100, 200, 300, 400};
            data.setHighscores(initialScores);
            dm.setGamedata(data);
            ASSERT_TRUE(dm.saveGame());

            // Update high scores and save again
            GameData updatedData = dm.getGamedata();
            std::array<std::size_t, 4> updatedScores = {500, 600, 700, 800};
            updatedData.setHighscores(updatedScores);
            dm.setGamedata(updatedData);
            ASSERT_TRUE(dm.saveGame());

            // Load in a new manager
            DataManager dm2;
            dm2.init(m_testFilename);

            const GameData &loadedData = dm2.getGamedata();

            // Check the updated scores were saved
            ASSERT_EQ(loadedData.getNickname(), "TestUser");
            ASSERT_EQ(loadedData.getHighscores(), updatedScores);
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

    TEST_F(DataManagerTest, NewGame)
    {
        try
        {
            DataManager dm;
            dm.init(m_testFilename);

            // Set initial data
            GameData data;
            data.setNickname("TestUser");
            std::array<std::size_t, 4> scores = {100, 200, 300, 400};
            data.setHighscores(scores);
            dm.setGamedata(data);
            dm.saveGame();

            // Create a new game
            dm.newGame();

            const GameData &newData = dm.getGamedata();

            // Check reset to default values
            ASSERT_EQ(newData.getNickname(), "");
            std::array<std::size_t, 4> expectedScores = {0, 0, 0, 0};
            ASSERT_EQ(newData.getHighscores(), expectedScores);

            // Check that original saved data still exists on disk
            DataManager dm2;
            dm2.init(m_testFilename);

            const GameData &loadedData = dm2.getGamedata();
            ASSERT_EQ(loadedData.getNickname(), "TestUser");
            ASSERT_EQ(loadedData.getHighscores(), scores);
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

    TEST_F(DataManagerTest, LoadGameFail)
    {
        try
        {
            DataManager dm;
            dm.init("non_existent_file.json"); // Test loading a non-existent file

            // Should initialize with default empty values
            const GameData &newData = dm.getGamedata();
            ASSERT_EQ(newData.getNickname(), "");
            std::array<std::size_t, 4> expectedScores = {0, 0, 0, 0};
            ASSERT_EQ(newData.getHighscores(), expectedScores);
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

    TEST_F(DataManagerTest, GuestModeNoSave)
    {
        try
        {
            DataManager dm;
            dm.init(m_testFilename);

            // Set empty nickname for guest mode
            GameData data;
            data.setNickname(""); // Empty nickname represents guest mode
            std::array<std::size_t, 4> scores = {500, 600, 700, 800};
            data.setHighscores(scores);
            dm.setGamedata(data);

            // Should return true but not create a file
            ASSERT_TRUE(dm.saveGame());
            ASSERT_FALSE(std::filesystem::exists(m_testFilename)) << "File should not be created for guest mode";
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

    TEST_F(DataManagerTest, HighscoreUpdating)
    {
        try
        {
            DataManager dm;
            dm.init(m_testFilename);

            // Set initial data
            GameData initialData;
            initialData.setNickname("Player1");
            std::array<std::size_t, 4> initialScores = {100, 200, 300, 400};
            initialData.setHighscores(initialScores);
            dm.setGamedata(initialData);
            dm.saveGame();

            // Set lower scores
            GameData lowerData = dm.getGamedata();
            std::array<std::size_t, 4> lowerScores = {50, 150, 250, 350};
            lowerData.setHighscores(lowerScores);
            dm.setGamedata(lowerData);
            dm.saveGame();

            // Load data and verify the lower scores were saved
            DataManager dm2;
            dm2.init(m_testFilename);
            ASSERT_EQ(dm2.getGamedata().getHighscores(), lowerScores);

            // Set higher scores
            GameData higherData = dm2.getGamedata();
            std::array<std::size_t, 4> higherScores = {200, 300, 400, 500};
            higherData.setHighscores(higherScores);
            dm2.setGamedata(higherData);
            dm2.saveGame();

            // Load again and verify
            DataManager dm3;
            dm3.init(m_testFilename);
            ASSERT_EQ(dm3.getGamedata().getHighscores(), higherScores);
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

    TEST_F(DataManagerTest, EncryptionTransition)
    {
        try
        {
            // Create an unencrypted save file
            {
                DataManager dm;
                dm.init(m_testFilename, false); // Unencrypted

                GameData data;
                data.setNickname("TransitionTest");
                std::array<std::size_t, 4> scores = {1000, 2000, 3000, 4000};
                data.setHighscores(scores);
                dm.setGamedata(data);

                ASSERT_TRUE(dm.saveGame()) << "Failed to save unencrypted game";

                // Verify it's unencrypted
                ASSERT_FALSE(DataReaderWriter::isFileEncrypted(m_testFilename));
            }

            // Load the file with encryption turned on
            {
                DataManager dm;
                dm.init(m_testFilename, true); // Encrypted mode

                // Should still load successfully due to auto-detection
                const GameData &data = dm.getGamedata();
                ASSERT_EQ(data.getNickname(), "TransitionTest");
                std::array<std::size_t, 4> expectedScores = {1000, 2000, 3000, 4000};
                ASSERT_EQ(data.getHighscores(), expectedScores);

                // Modify data
                GameData updatedData = data;
                std::array<std::size_t, 4> updatedScores = {2000, 3000, 4000, 5000};
                updatedData.setHighscores(updatedScores);
                dm.setGamedata(updatedData);

                // Save with encryption turned on
                ASSERT_TRUE(dm.saveGame()) << "Failed to save with encryption";

                // Verify file is now encrypted
                ASSERT_TRUE(DataReaderWriter::isFileEncrypted(m_testFilename));
            }

            // Load the now-encrypted file with encryption turned off
            {
                DataManager dm;
                dm.init(m_testFilename, false); // Unencrypted mode

                // Should still load successfully due to auto-detection
                const GameData &data = dm.getGamedata();
                ASSERT_EQ(data.getNickname(), "TransitionTest");
                std::array<std::size_t, 4> expectedScores = {2000, 3000, 4000, 5000};
                ASSERT_EQ(data.getHighscores(), expectedScores);
            }
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

    TEST_F(DataManagerTest, SetEncryptionDuringOperation)
    {
        try
        {
            // Create an unencrypted save file
            {
                DataManager dm;
                dm.init(m_testFilename, false); // Unencrypted

                GameData data;
                data.setNickname("EncryptionChangeTest");
                std::array<std::size_t, 4> scores = {100, 200, 300, 400};
                data.setHighscores(scores);
                dm.setGamedata(data);

                ASSERT_TRUE(dm.saveGame()) << "Failed to save unencrypted game";

                // Verify it's unencrypted
                ASSERT_FALSE(DataReaderWriter::isFileEncrypted(m_testFilename));

                // Change encryption setting mid-operation
                dm.setEncryption(true);

                // Update data
                GameData updatedData = dm.getGamedata();
                std::array<std::size_t, 4> updatedScores = {200, 300, 400, 500};
                updatedData.setHighscores(updatedScores);
                dm.setGamedata(updatedData);

                // Save with new encryption setting
                ASSERT_TRUE(dm.saveGame()) << "Failed to save with changed encryption";

                // Verify file is now encrypted
                ASSERT_TRUE(DataReaderWriter::isFileEncrypted(m_testFilename));
            }

            // Load the now-encrypted file with correct setting
            {
                DataManager dm;
                dm.init(m_testFilename, true); // Encrypted mode

                // Should load successfully
                const GameData &data = dm.getGamedata();
                ASSERT_EQ(data.getNickname(), "EncryptionChangeTest");
                std::array<std::size_t, 4> expectedScores = {200, 300, 400, 500};
                ASSERT_EQ(data.getHighscores(), expectedScores);
            }
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

    TEST_F(DataManagerTest, IsEncryptedMethod)
    {
        try
        {
            // Test initial state after construction - file doesn't exist yet
            {
                DataManager dm;
                // isEncrypted should return m_fileEncrypted, which should be false initially
                ASSERT_FALSE(dm.isEncrypted()) << "File encryption state should initially be false";
            }

            // Create an encrypted file and test if isEncrypted returns true
            {
                // Create and save an encrypted file
                DataManager dm;
                dm.init(m_testFilename, true); // Use encryption

                GameData data;
                data.setNickname("EncryptedTest");
                std::array<std::size_t, 4> scores = {100, 200, 300, 400};
                data.setHighscores(scores);
                dm.setGamedata(data);

                ASSERT_TRUE(dm.saveGame());

                // Now verify the file is encrypted and isEncrypted() returns true
                ASSERT_TRUE(DataReaderWriter::isFileEncrypted(m_testFilename));
                ASSERT_TRUE(dm.isEncrypted()) << "isEncrypted should return true after saving encrypted file";
            }

            // Create an unencrypted file and test if isEncrypted returns false
            {
                // Clear existing file to force creating a new one
                std::filesystem::remove(m_testFilename);

                // Create unencrypted file
                DataManager dm;
                dm.init(m_testFilename, false); // No encryption

                GameData data;
                data.setNickname("UnencryptedTest");
                std::array<std::size_t, 4> scores = {200, 300, 400, 500};
                data.setHighscores(scores);
                dm.setGamedata(data);

                ASSERT_TRUE(dm.saveGame());

                // Now verify the file is not encrypted and isEncrypted() returns false
                ASSERT_FALSE(DataReaderWriter::isFileEncrypted(m_testFilename));
                ASSERT_FALSE(dm.isEncrypted()) << "isEncrypted should return false after saving unencrypted file";
            }

            // Test loading an encrypted file - isEncrypted should return true
            {
                // First create an encrypted file
                {
                    // Clear existing file
                    std::filesystem::remove(m_testFilename);

                    DataManager dm;
                    dm.init(m_testFilename, true); // Encryption on

                    GameData data;
                    data.setNickname("EncryptionStateTest");
                    std::array<std::size_t, 4> scores = {300, 400, 500, 600};
                    data.setHighscores(scores);
                    dm.setGamedata(data);

                    ASSERT_TRUE(dm.saveGame());
                }

                // Now load it with a new DataManager and check isEncrypted
                {
                    DataManager dm;
                    dm.init(m_testFilename, true); // Match file encryption
                    ASSERT_TRUE(dm.isEncrypted()) << "isEncrypted should reflect the file's encryption state after loading";
                }
            }

            // Test changing encryption with setEncryption and saving
            {
                // Set up initial state - encrypted file
                {
                    // Clear existing file
                    std::filesystem::remove(m_testFilename);

                    DataManager dm;
                    dm.init(m_testFilename, true); // Start with encryption

                    GameData data;
                    data.setNickname("ChangeEncryptionTest");
                    std::array<std::size_t, 4> scores = {300, 400, 500, 600};
                    data.setHighscores(scores);
                    dm.setGamedata(data);

                    ASSERT_TRUE(dm.saveGame());
                }

                // Load file, change encryption setting, and save
                {
                    DataManager dm;
                    dm.init(m_testFilename, true); // Load encrypted file

                    // Initial state should be encrypted
                    ASSERT_TRUE(dm.isEncrypted());

                    // Change encryption setting and save
                    dm.setEncryption(false);

                    // Get data, update it, and set it back
                    GameData data = dm.getGamedata();
                    std::array<std::size_t, 4> updatedScores = {400, 500, 600, 700};
                    data.setHighscores(updatedScores);
                    dm.setGamedata(data);

                    ASSERT_TRUE(dm.saveGame()); // Save with new encryption setting

                    // After saving, isEncrypted should reflect the new state
                    ASSERT_FALSE(dm.isEncrypted()) << "isEncrypted should return false after saving with encryption off";
                    ASSERT_FALSE(DataReaderWriter::isFileEncrypted(m_testFilename));
                }
            }
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }
} // namespace datacoe