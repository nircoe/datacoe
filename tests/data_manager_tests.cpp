#include "gtest/gtest.h"
#include "data_manager.hpp"
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
            dm.init(m_testFilename);

            dm.setNickName("TestUser");
            dm.setHighScore(100);

            ASSERT_TRUE(dm.saveGame()) << "Failed to save game";
            ASSERT_TRUE(std::filesystem::exists(m_testFilename)) << "Save file not created";

            // Create a new DataManager to load the saved data
            DataManager dm2;
            dm2.init(m_testFilename);

            const GameData &loadedData = dm2.getGameData();

            ASSERT_EQ(loadedData.getNickName(), "TestUser");
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
            dm.init(m_testFilename);

            // Set initial data and save
            dm.setNickName("TestUser");
            dm.setHighScore(100);
            ASSERT_TRUE(dm.saveGame());

            // Update high score and save again
            dm.setHighScore(200);
            ASSERT_TRUE(dm.saveGame());

            // Load in a new manager
            DataManager dm2;
            dm2.init(m_testFilename);

            const GameData &loadedData = dm2.getGameData();

            // Check the updated score was saved
            ASSERT_EQ(loadedData.getNickName(), "TestUser");
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
            dm.init(m_testFilename);
            dm.setNickName("TestUser");
            dm.setHighScore(100);
            dm.saveGame();

            // Create a new game
            dm.newGame();

            const GameData &newData = dm.getGameData();

            // Check reset to default values
            ASSERT_EQ(newData.getNickName(), "");
            ASSERT_EQ(newData.getHighscore(), 0);

            // Check that original saved data still exists on disk
            DataManager dm2;
            dm2.init(m_testFilename);

            const GameData &loadedData = dm2.getGameData();
            ASSERT_EQ(loadedData.getNickName(), "TestUser");
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
            dm.init("non_existent_file.json"); // Test loading a non-existent file

            // Should initialize with default empty values
            const GameData &newData = dm.getGameData();
            ASSERT_EQ(newData.getNickName(), "");
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
            dm.init(m_testFilename);

            // Empty nickname represents guest mode
            dm.setNickName("");
            dm.setHighScore(500);

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
            dm.init(m_testFilename);

            dm.setNickName("Player1");
            dm.setHighScore(100);
            dm.saveGame();

            // Set a lower score
            dm.setHighScore(50);
            dm.saveGame();

            // Load data and verify the lower score was saved
            DataManager dm2;
            dm2.init(m_testFilename);
            ASSERT_EQ(dm2.getGameData().getHighscore(), 50);

            // Set a higher score
            dm2.setHighScore(200);
            dm2.saveGame();

            // Load again and verify
            DataManager dm3;
            dm3.init(m_testFilename);
            ASSERT_EQ(dm3.getGameData().getHighscore(), 200);
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
                dm.setNickName("TransitionTest");
                dm.setHighScore(1000);
                ASSERT_TRUE(dm.saveGame()) << "Failed to save unencrypted game";

                // Verify it's unencrypted
                ASSERT_FALSE(DataReaderWriter::isFileEncrypted(m_testFilename));
            }

            // Load the file with encryption turned on
            {
                DataManager dm;
                dm.init(m_testFilename, true); // Encrypted mode

                // Should still load successfully due to auto-detection
                const GameData &data = dm.getGameData();
                ASSERT_EQ(data.getNickName(), "TransitionTest");
                ASSERT_EQ(data.getHighscore(), 1000);

                // Modify data
                dm.setHighScore(2000);

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
                const GameData &data = dm.getGameData();
                ASSERT_EQ(data.getNickName(), "TransitionTest");
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
                dm.setNickName("EncryptionChangeTest");
                dm.setHighScore(100);
                ASSERT_TRUE(dm.saveGame()) << "Failed to save unencrypted game";

                // Verify it's unencrypted
                ASSERT_FALSE(DataReaderWriter::isFileEncrypted(m_testFilename));

                // Change encryption setting mid-operation
                dm.setEncryption(true);

                // Update data
                dm.setHighScore(200);

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
                const GameData &data = dm.getGameData();
                ASSERT_EQ(data.getNickName(), "EncryptionChangeTest");
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
                dm.setNickName("EncryptedTest");
                dm.setHighScore(100);
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
                dm.setNickName("UnencryptedTest");
                dm.setHighScore(200);
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
                    dm.setNickName("EncryptionStateTest");
                    dm.setHighScore(300);
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
                    dm.setNickName("ChangeEncryptionTest");
                    dm.setHighScore(300);
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
                    dm.setHighScore(400);       // Change data
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