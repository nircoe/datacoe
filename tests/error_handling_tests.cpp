#include <gtest/gtest.h>
#include <datacoe/data_manager.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace datacoe
{
    class ErrorHandlingTest : public ::testing::Test
    {
    protected:
        std::string m_testFilename;
        std::string m_corruptFilename;

        void SetUp() override
        {
            m_testFilename = "error_test_data.json";
            m_corruptFilename = "corrupt_test_data.json";
            // Clean up any leftover files from previous tests
            try
            {
                if (std::filesystem::exists(m_testFilename))
                {
                    std::filesystem::remove(m_testFilename);
                }
                if (std::filesystem::exists(m_corruptFilename))
                {
                    std::filesystem::remove(m_corruptFilename);
                }
            }
            catch (const std::filesystem::filesystem_error &)
            {
                // Ignore errors if file doesn't exist
            }
        }

        void TearDown() override
        {
            // Wait longer to ensure file handles are released
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            try
            {
                for (int retries = 0; retries < 5; retries++)
                {
                    if (std::filesystem::exists(m_testFilename))
                    {
                        std::error_code ec;
                        std::filesystem::remove(m_testFilename, ec);
                        if (!ec)
                            break;

                        std::cerr << "Failed to remove file on attempt " << retries
                                  << ": " << ec.message() << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    else
                    {
                        break;
                    }
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Exception during TearDown: " << e.what() << std::endl;
            }
        }

        // Helper to create a corrupted file
        void createCorruptJsonFile()
        {
            // First create a valid file
            {
                DataManager dm;
                bool initResult = dm.init(m_corruptFilename);
                ASSERT_FALSE(initResult) << "init() should return false for new file";

                GameData gameData;
                gameData.setNickname("ValidData");
                gameData.setHighscore(500);
                dm.setGamedata(gameData);

                bool saveResult = dm.saveGame();
                ASSERT_TRUE(saveResult) << "Failed to create initial file for corruption test";
            }

            // Now corrupt it by appending invalid JSON
            std::ofstream file(m_corruptFilename, std::ios::app);
            ASSERT_TRUE(file.is_open()) << "Failed to open file for corruption";
            file << "this is not valid json";
            file.close();
        }

        // Helper to create a permission-denied file
        bool createReadOnlyFile()
        {
#ifdef _WIN32
            // Create file
            {
                DataManager dm;
                dm.init(m_testFilename);

                GameData gameData;
                gameData.setNickname("ReadOnly");
                gameData.setHighscore(100);
                dm.setGamedata(gameData);

                bool saveResult = dm.saveGame();
                if (!saveResult)
                {
                    return false;
                }
            }

            // Make it read-only
            std::string command = "attrib +r " + m_testFilename;
            if (system(command.c_str()) != 0)
            {
                return false;
            }
            return true;
#else
            // Create file
            {
                DataManager dm;
                dm.init(m_testFilename);

                GameData gameData;
                gameData.setNickname("ReadOnly");
                gameData.setHighscore(100);
                dm.setGamedata(gameData);

                bool saveResult = dm.saveGame();
                if (!saveResult)
                {
                    return false;
                }
            }

            // Make it read-only
            if (chmod(m_testFilename.c_str(), S_IRUSR | S_IRGRP | S_IROTH) != 0)
            {
                return false;
            }
            return true;
#endif
        }

        // Helper to restore writability
        void restoreWritePermission()
        {
#ifdef _WIN32
            std::string command = "attrib -r " + m_testFilename;
            system(command.c_str());
#else
            chmod(m_testFilename.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif
        }
    };

    TEST_F(ErrorHandlingTest, CorruptJsonFile)
    {
        createCorruptJsonFile();

        // Attempt to load corrupt data
        DataManager dm;
        bool initResult = dm.init(m_corruptFilename);
        ASSERT_FALSE(initResult) << "init() should return false for corrupted file";

        // Should either initialize with defaults or throw an exception
        // Either way, the manager should be in a valid state

        // Just check that we can still use the manager
        GameData gameData;
        gameData.setNickname("RecoveredData");
        gameData.setHighscore(999);
        dm.setGamedata(gameData);
        bool saveResult = dm.saveGame();
        ASSERT_TRUE(saveResult) << "Failed to save after recovery";

        // Try loading again - should work now
        DataManager dm2;
        bool loadResult = dm2.init(m_corruptFilename);
        ASSERT_TRUE(loadResult) << "init() should return true after file is repaired";
        ASSERT_EQ(dm2.getGamedata().getNickname(), "RecoveredData");
        ASSERT_EQ(dm2.getGamedata().getHighscore(), 999);
    }

    TEST_F(ErrorHandlingTest, NonExistentDirectory)
    {
        std::string nonExistentPath = "non/existent/directory/file.json";

        DataManager dm;
        bool initResult = dm.init(nonExistentPath);
        ASSERT_FALSE(initResult) << "init() should return false for non-existent directory";

        // Should still be able to set data
        GameData gameData;
        gameData.setNickname("TestNonExistent");
        gameData.setHighscore(123);
        dm.setGamedata(gameData);

        // Save will likely fail but shouldn't crash
        bool saveResult = dm.saveGame();

        // We don't assert on saveResult because it depends on implementation
        // The key is that the call doesn't crash

        std::cout << "Save to non-existent directory "
                  << (saveResult ? "succeeded" : "failed as expected") << std::endl;
    }

    TEST_F(ErrorHandlingTest, EmptyFilename)
    {
        DataManager dm;
        bool initResult = dm.init(""); // Empty filename
        ASSERT_FALSE(initResult) << "init() should return false for empty filename";

        // Should still be able to use the manager
        GameData gameData;
        gameData.setNickname("EmptyFilename");
        gameData.setHighscore(123);
        dm.setGamedata(gameData);

        // Save may fail but shouldn't crash
        bool saveResult = dm.saveGame();
        std::cout << "Save with empty filename "
                  << (saveResult ? "succeeded" : "failed as expected") << std::endl;
    }

    TEST_F(ErrorHandlingTest, ReadOnlyFile)
    {
        // Skip if we can't make a read-only file
        if (!createReadOnlyFile())
        {
            GTEST_SKIP() << "Cannot create read-only file for testing";
        }

        // Try to save to a read-only file
        DataManager dm;
        bool initResult = dm.init(m_testFilename);
        ASSERT_TRUE(initResult) << "init() should return true when loading existing file";

        GameData gameData;
        gameData.setNickname("NewData");
        gameData.setHighscore(200);
        dm.setGamedata(gameData);

        // Save will likely fail but shouldn't crash
        bool saveResult = dm.saveGame();
        std::cout << "Save to read-only file "
                  << (saveResult ? "succeeded unexpectedly" : "failed as expected") << std::endl;

        // Restore write permission for cleanup
        restoreWritePermission();
    }

    TEST_F(ErrorHandlingTest, InterruptedSave)
    {
        // Test handling of partial/interrupted saves

        // First create valid data
        {
            DataManager dm;
            bool initResult = dm.init(m_testFilename);
            ASSERT_FALSE(initResult) << "init() should return false for new file";

            GameData gameData;
            gameData.setNickname("Original");
            gameData.setHighscore(100);
            dm.setGamedata(gameData);

            bool saveResult = dm.saveGame();
            ASSERT_TRUE(saveResult) << "Failed to save initial data";
        }

        // Simulate interrupted save by truncating file
        {
            std::ofstream file(m_testFilename, std::ios::out | std::ios::trunc);
            file << "{"; // Just open the JSON but don't close it - corrupt state
            file.close();
        }

        // Try to load the truncated file
        DataManager dm;
        bool initResult = dm.init(m_testFilename);
        ASSERT_FALSE(initResult) << "init() should return false for truncated file";

        // Manager should still be usable
        GameData gameData;
        gameData.setNickname("Recovered");
        gameData.setHighscore(200);
        dm.setGamedata(gameData);
        bool saveResult = dm.saveGame();
        ASSERT_TRUE(saveResult) << "Failed to save after recovery";

        // Verify recovery worked
        DataManager dm2;
        bool loadResult = dm2.init(m_testFilename);
        ASSERT_TRUE(loadResult) << "init() should return true for repaired file";
        ASSERT_EQ(dm2.getGamedata().getNickname(), "Recovered");
        ASSERT_EQ(dm2.getGamedata().getHighscore(), 200);
    }

    TEST_F(ErrorHandlingTest, MalformedJson)
    {
        // Create a file with valid JSON but wrong structure
        {
            std::ofstream file(m_testFilename);
            file << R"({"wrongKey": "wrongValue"})";
            file.close();
        }

        // Try to load the malformed file
        DataManager dm;
        bool initResult = dm.init(m_testFilename);
        ASSERT_FALSE(initResult) << "init() should return false for malformed JSON";

        // Verify we can save valid data
        GameData gameData;
        gameData.setNickname("FixedData");
        gameData.setHighscore(300);
        dm.setGamedata(gameData);
        bool saveResult = dm.saveGame();
        ASSERT_TRUE(saveResult) << "Failed to save after malformed JSON recovery";

        // Check that the data was saved correctly
        DataManager dm2;
        bool loadResult = dm2.init(m_testFilename);
        ASSERT_TRUE(loadResult) << "init() should return true after saving valid data";
        ASSERT_EQ(dm2.getGamedata().getNickname(), "FixedData");
        ASSERT_EQ(dm2.getGamedata().getHighscore(), 300);
    }

} // namespace datacoe