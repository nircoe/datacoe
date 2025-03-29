#include <gtest/gtest.h>
#include <datacoe/data_manager.hpp>
#include <filesystem>
#include <memory>
#include <vector>

namespace datacoe
{
    class MemoryTest : public ::testing::Test
    {
    protected:
        std::string m_testFilename;

        void SetUp() override
        {
            m_testFilename = "memory_test_data.json";
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
            try
            {
                if (std::filesystem::exists(m_testFilename))
                {
                    std::filesystem::remove(m_testFilename);
                }
            }
            catch (const std::filesystem::filesystem_error &)
            {
                // Ignore errors
            }
        }
    };

    TEST_F(MemoryTest, RepeatedCreationAndDestruction)
    {
        // This test checks for memory leaks by repeatedly creating and destroying objects
        constexpr int iterations = 1000;

        for (int i = 0; i < iterations; i++)
        {
            DataManager dm;
            bool initResult = dm.init(m_testFilename);

            // First iteration shouldn't find a file
            if (i == 0)
                ASSERT_FALSE(initResult) << "First init() should return false for new file";
            else
                ASSERT_TRUE(initResult) << "Subsequent init() calls should return true for existing file";

            GameData gameData;
            gameData.setNickname("MemoryTest");
            gameData.setHighscore(i);
            dm.setGamedata(gameData);

            dm.saveGame();
        }

        // If there's a memory leak, this test will consume a lot of memory
        // Tools like Valgrind or Address Sanitizer should be used to detect actual leaks

        // Verify functionality still works
        DataManager finalDm;
        bool finalLoadResult = finalDm.init(m_testFilename);
        ASSERT_TRUE(finalLoadResult) << "Final init() should return true for existing file";
        ASSERT_EQ(finalDm.getGamedata().getNickname(), "MemoryTest");
        ASSERT_EQ(finalDm.getGamedata().getHighscore(), iterations - 1);
    }

    TEST_F(MemoryTest, LargeDataHandling)
    {
        // Test with larger-than-typical data to check memory handling
        constexpr int dataSize = 1000; // Large enough to test memory usage but not too large

        // Create a DataManager with a large nickname
        {
            DataManager dm;
            bool initResult = dm.init(m_testFilename);
            ASSERT_FALSE(initResult) << "init() should return false for new file";

            // Generate a large string
            std::string largeString(dataSize, 'A');

            GameData gameData;
            gameData.setNickname(largeString);
            gameData.setHighscore(999999);
            dm.setGamedata(gameData);

            ASSERT_TRUE(dm.saveGame());
        }

        // Check data was saved correctly
        {
            DataManager dm;
            bool loadResult = dm.init(m_testFilename);
            ASSERT_TRUE(loadResult) << "init() should return true for existing file";
            ASSERT_EQ(dm.getGamedata().getNickname().size(), dataSize);
            ASSERT_EQ(dm.getGamedata().getHighscore(), 999999);
        }
    }

    TEST_F(MemoryTest, MultipleInstancesWithSameFile)
    {
        // Test multiple DataManager instances using the same file
        constexpr int instanceCount = 10;

        // Create initial data
        {
            DataManager dm;
            bool initResult = dm.init(m_testFilename);
            ASSERT_FALSE(initResult) << "init() should return false for new file";

            GameData gameData;
            gameData.setNickname("InitialData");
            gameData.setHighscore(1000);
            dm.setGamedata(gameData);

            ASSERT_TRUE(dm.saveGame());
        }

        // Create multiple instances all pointing to the same file
        std::vector<std::unique_ptr<DataManager>> managers;
        for (int i = 0; i < instanceCount; i++)
        {
            auto dm = std::make_unique<DataManager>();
            bool loadResult = dm->init(m_testFilename);
            ASSERT_TRUE(loadResult) << "init() should return true for existing file";
            managers.push_back(std::move(dm));
        }

        // Have each manager modify the data
        for (int i = 0; i < instanceCount; i++)
        {
            GameData gameData;
            gameData.setNickname("Manager" + std::to_string(i));
            gameData.setHighscore(2000 + i);
            managers[i]->setGamedata(gameData);
            ASSERT_TRUE(managers[i]->saveGame());
        }

        // Check the final state
        {
            DataManager dm;
            bool loadResult = dm.init(m_testFilename);
            ASSERT_TRUE(loadResult) << "init() should return true for existing file";
            ASSERT_EQ(dm.getGamedata().getNickname(), "Manager" + std::to_string(instanceCount - 1));
            ASSERT_EQ(dm.getGamedata().getHighscore(), 2000 + instanceCount - 1);
        }

        // Release all managers
        managers.clear();

        // Verify file access still works
        {
            DataManager dm;
            bool finalLoadResult = dm.init(m_testFilename);
            ASSERT_TRUE(finalLoadResult) << "init() should return true for existing file";
            const GameData &data = dm.getGamedata();
            ASSERT_FALSE(data.getNickname().empty());
        }
    }
} // namespace datacoe