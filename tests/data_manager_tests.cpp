#include "gtest/gtest.h"
#include "datacoe/data_manager.hpp"
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

    TEST_F(DataManagerTest, SaveAndLoadGame)
    {
        try
        {
            DataManager dm;
            bool initResult = dm.init(m_testFilename);
            ASSERT_FALSE(initResult) << "init() should return false for new file";

            // Create a GameData object and set it in the DataManager
            GameData gameData;
            gameData.setNickname("TestUser");
            gameData.setHighscore(100);
            dm.setGamedata(gameData);

            ASSERT_TRUE(dm.saveGame()) << "Failed to save game";
            ASSERT_TRUE(std::filesystem::exists(m_testFilename)) << "Save file not created";

            // Create a new DataManager to load the saved data
            DataManager dm2;
            bool loadResult = dm2.init(m_testFilename);
            ASSERT_TRUE(loadResult) << "init() should return true when loading existing file";

            const GameData &loadedData = dm2.getGamedata();

            ASSERT_EQ(loadedData.getNickname(), "TestUser");
            ASSERT_EQ(loadedData.getHighscore(), 100);
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
            bool initialInit = dm.init(m_testFilename);
            ASSERT_FALSE(initialInit) << "init() should return false for new file";

            // Set initial data and save
            GameData gameData;
            gameData.setNickname("TestUser");
            gameData.setHighscore(100);
            dm.setGamedata(gameData);
            ASSERT_TRUE(dm.saveGame());

            // Update high score and save again
            GameData updatedData = dm.getGamedata(); // Get the current data
            updatedData.setHighscore(200);           // Update the score
            dm.setGamedata(updatedData);             // Set the updated data
            ASSERT_TRUE(dm.saveGame());

            // Load in a new manager
            DataManager dm2;
            bool loadResult = dm2.init(m_testFilename);
            ASSERT_TRUE(loadResult) << "init() should return true when loading existing file";

            const GameData &loadedData = dm2.getGamedata();

            // Check the updated score was saved
            ASSERT_EQ(loadedData.getNickname(), "TestUser");
            ASSERT_EQ(loadedData.getHighscore(), 200);
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
            bool initResult = dm.init(m_testFilename);
            ASSERT_FALSE(initResult) << "init() should return false for new file";

            // Create initial data
            GameData gameData;
            gameData.setNickname("TestUser");
            gameData.setHighscore(100);
            dm.setGamedata(gameData);
            dm.saveGame();

            // Create a new game
            dm.newGame();

            const GameData &newData = dm.getGamedata();

            // Check reset to default values
            ASSERT_EQ(newData.getNickname(), "");
            ASSERT_EQ(newData.getHighscore(), 0);

            // Check that original saved data still exists on disk
            DataManager dm2;
            bool loadResult = dm2.init(m_testFilename);
            ASSERT_TRUE(loadResult) << "init() should return true when loading existing file";

            const GameData &loadedData = dm2.getGamedata();
            ASSERT_EQ(loadedData.getNickname(), "TestUser");
            ASSERT_EQ(loadedData.getHighscore(), 100);
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
            bool initResult = dm.init("non_existent_file.json"); // Test loading a non-existent file

            // Should return false for non-existent file
            ASSERT_FALSE(initResult) << "init() should return false for non-existent file";

            // Should initialize with default empty values
            const GameData &newData = dm.getGamedata();
            ASSERT_EQ(newData.getNickname(), "");
            ASSERT_EQ(newData.getHighscore(), 0);
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
            bool initResult = dm.init(m_testFilename);
            ASSERT_FALSE(initResult) << "init() should return false for new file";

            // Empty nickname represents guest mode
            GameData gameData;
            gameData.setNickname(""); // Empty nickname for guest mode
            gameData.setHighscore(500);
            dm.setGamedata(gameData);

            // Should return true but not create a file
            ASSERT_TRUE(dm.saveGame());
            ASSERT_FALSE(std::filesystem::exists(m_testFilename)) << "File should not be created for guest mode";
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

    TEST_F(DataManagerTest, HighScoreUpdating)
    {
        try
        {
            DataManager dm;
            bool initResult = dm.init(m_testFilename);
            ASSERT_FALSE(initResult) << "init() should return false for new file";

            // Set initial data
            GameData gameData;
            gameData.setNickname("Player1");
            gameData.setHighscore(100);
            dm.setGamedata(gameData);
            dm.saveGame();

            // Set a lower score
            GameData updatedData = dm.getGamedata();
            updatedData.setHighscore(50);
            dm.setGamedata(updatedData);
            dm.saveGame();

            // Load data and verify the lower score was saved
            DataManager dm2;
            bool loadResult1 = dm2.init(m_testFilename);
            ASSERT_TRUE(loadResult1) << "init() should return true when loading existing file";
            ASSERT_EQ(dm2.getGamedata().getHighscore(), 50);

            // Set a higher score
            GameData newData = dm2.getGamedata();
            newData.setHighscore(200);
            dm2.setGamedata(newData);
            dm2.saveGame();

            // Load again and verify
            DataManager dm3;
            bool loadResult2 = dm3.init(m_testFilename);
            ASSERT_TRUE(loadResult2) << "init() should return true when loading existing file";
            ASSERT_EQ(dm3.getGamedata().getHighscore(), 200);
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
                bool initResult = dm.init(m_testFilename, false); // Unencrypted
                ASSERT_FALSE(initResult) << "init() should return false for new file";

                GameData gameData;
                gameData.setNickname("TransitionTest");
                gameData.setHighscore(1000);
                dm.setGamedata(gameData);
                ASSERT_TRUE(dm.saveGame()) << "Failed to save unencrypted game";

                // Verify it's unencrypted
                ASSERT_FALSE(DataReaderWriter::isFileEncrypted(m_testFilename));
            }

            // Load the file with encryption turned on
            {
                DataManager dm;
                bool loadResult = dm.init(m_testFilename, true); // Encrypted mode
                ASSERT_TRUE(loadResult) << "init() should return true when loading existing file";

                // Should still load successfully due to auto-detection
                const GameData &data = dm.getGamedata();
                ASSERT_EQ(data.getNickname(), "TransitionTest");
                ASSERT_EQ(data.getHighscore(), 1000);

                // Modify data
                GameData updatedData = dm.getGamedata();
                updatedData.setHighscore(2000);
                dm.setGamedata(updatedData);

                // Save with encryption turned on
                ASSERT_TRUE(dm.saveGame()) << "Failed to save with encryption";

                // Verify file is now encrypted
                ASSERT_TRUE(DataReaderWriter::isFileEncrypted(m_testFilename));
            }

            // Load the now-encrypted file with encryption turned off
            {
                DataManager dm;
                bool loadResult = dm.init(m_testFilename, false); // Unencrypted mode
                ASSERT_TRUE(loadResult) << "init() should return true when loading existing file (with auto-detection)";

                // Should still load successfully due to auto-detection
                const GameData &data = dm.getGamedata();
                ASSERT_EQ(data.getNickname(), "TransitionTest");
                ASSERT_EQ(data.getHighscore(), 2000);
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
                GameData gameData;
                gameData.setNickname("EncryptionChangeTest");
                gameData.setHighscore(100);
                dm.setGamedata(gameData);
                ASSERT_TRUE(dm.saveGame()) << "Failed to save unencrypted game";

                // Verify it's unencrypted
                ASSERT_FALSE(DataReaderWriter::isFileEncrypted(m_testFilename));

                // Change encryption setting mid-operation
                dm.setEncryption(true);

                // Update data
                GameData updatedData = dm.getGamedata();
                updatedData.setHighscore(200);
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
                ASSERT_EQ(data.getHighscore(), 200);
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
                GameData gameData;
                gameData.setNickname("EncryptedTest");
                gameData.setHighscore(100);
                dm.setGamedata(gameData);
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
                GameData gameData;
                gameData.setNickname("UnencryptedTest");
                gameData.setHighscore(200);
                dm.setGamedata(gameData);
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
                    GameData gameData;
                    gameData.setNickname("EncryptionStateTest");
                    gameData.setHighscore(300);
                    dm.setGamedata(gameData);
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
                    GameData gameData;
                    gameData.setNickname("ChangeEncryptionTest");
                    gameData.setHighscore(300);
                    dm.setGamedata(gameData);
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
                    GameData gameData = dm.getGamedata();
                    gameData.setHighscore(400); // Change data
                    dm.setGamedata(gameData);
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