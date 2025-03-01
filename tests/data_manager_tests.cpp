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
        void SetUp() override
        {
            testFilename = "test_data_manager.json";
            // Clean up any leftover files from previous tests
            try
            {
                if (std::filesystem::exists(testFilename))
                {
                    std::filesystem::remove(testFilename);
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
                    if (std::filesystem::exists(testFilename))
                    {
                        std::filesystem::remove(testFilename);
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

        std::string testFilename;
    };

    TEST_F(DataManagerTest, SaveAndLoadGame)
    {
        try
        {
            DataManager dm;
            dm.init(testFilename);

            dm.setNickName("TestUser");
            dm.setHighScore(100);

            ASSERT_TRUE(dm.saveGame()) << "Failed to save game";
            ASSERT_TRUE(std::filesystem::exists(testFilename)) << "Save file not created";

            // Create a new DataManager to load the saved data
            DataManager dm2;
            dm2.init(testFilename);

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
            dm.init(testFilename);

            // Set initial data and save
            dm.setNickName("TestUser");
            dm.setHighScore(100);
            ASSERT_TRUE(dm.saveGame());

            // Update high score and save again
            dm.setHighScore(200);
            ASSERT_TRUE(dm.saveGame());

            // Load in a new manager
            DataManager dm2;
            dm2.init(testFilename);

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
            dm.init(testFilename);
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
            dm2.init(testFilename);

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
            dm.init(testFilename);

            // Empty nickname represents guest mode
            dm.setNickName("");
            dm.setHighScore(500);

            // Should return true but not create a file
            ASSERT_TRUE(dm.saveGame());
            ASSERT_FALSE(std::filesystem::exists(testFilename)) << "File should not be created for guest mode";
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
            dm.init(testFilename);

            dm.setNickName("Player1");
            dm.setHighScore(100);
            dm.saveGame();

            // Set a lower score
            dm.setHighScore(50);
            dm.saveGame();

            // Load data and verify the lower score was saved
            DataManager dm2;
            dm2.init(testFilename);
            ASSERT_EQ(dm2.getGameData().getHighscore(), 50);

            // Set a higher score
            dm2.setHighScore(200);
            dm2.saveGame();

            // Load again and verify
            DataManager dm3;
            dm3.init(testFilename);
            ASSERT_EQ(dm3.getGameData().getHighscore(), 200);
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

} // namespace datacoe