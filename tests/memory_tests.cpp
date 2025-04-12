#include "gtest/gtest.h"
#include "datacoe/data_manager.hpp"
#include <filesystem>
#include <fstream>
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
        constexpr std::size_t iterations = 1000;

        for (std::size_t i = 0; i < iterations; i++)
        {
            DataManager dm;
            dm.init(m_testFilename);

            GameData data;
            data.setNickname("MemoryTest");
            std::array<std::size_t, 4> scores = {i, i + 1, i + 2, i + 3};
            data.setHighscores(scores);
            dm.setGamedata(data);

            dm.saveGame();
        }

        // If there's a memory leak, this test will consume a lot of memory
        // Tools like Valgrind or Address Sanitizer should be used to detect actual leaks

        // Verify functionality still works
        DataManager finalDm;
        finalDm.init(m_testFilename);
        ASSERT_EQ(finalDm.getGamedata().getNickname(), "MemoryTest");
        std::array<std::size_t, 4> expectedScores = {iterations - 1, iterations, iterations + 1, iterations + 2};
        ASSERT_EQ(finalDm.getGamedata().getHighscores(), expectedScores);
    }

    TEST_F(MemoryTest, LargeDataHandling)
    {
        // Test with larger-than-typical data to check memory handling
        constexpr int dataSize = 1000; // Large enough to test memory usage but not too large

        // Create a DataManager with a large nickname
        {
            DataManager dm;
            dm.init(m_testFilename);

            // Generate a large string
            std::string largeString(dataSize, 'A');

            GameData data;
            data.setNickname(largeString);
            std::array<std::size_t, 4> scores = {999999, 999999, 999999, 999999};
            data.setHighscores(scores);
            dm.setGamedata(data);

            ASSERT_TRUE(dm.saveGame());
        }

        // Check data was saved correctly
        {
            DataManager dm;
            dm.init(m_testFilename);
            ASSERT_EQ(dm.getGamedata().getNickname().size(), dataSize);
            std::array<std::size_t, 4> expectedScores = {999999, 999999, 999999, 999999};
            ASSERT_EQ(dm.getGamedata().getHighscores(), expectedScores);
        }
    }

    TEST_F(MemoryTest, MultipleInstancesWithSameFile)
    {
        // Test multiple DataManager instances using the same file
        constexpr std::size_t instanceCount = 10;

        // Create initial data
        {
            DataManager dm;
            dm.init(m_testFilename);

            GameData data;
            data.setNickname("InitialData");
            std::array<std::size_t, 4> scores = {1000, 2000, 3000, 4000};
            data.setHighscores(scores);
            dm.setGamedata(data);

            ASSERT_TRUE(dm.saveGame());
        }

        // Create multiple instances all pointing to the same file
        std::vector<std::unique_ptr<DataManager>> managers;
        for (std::size_t i = 0; i < instanceCount; i++)
        {
            auto dm = std::make_unique<DataManager>();
            dm->init(m_testFilename);
            managers.push_back(std::move(dm));
        }

        // Have each manager modify the data
        for (std::size_t i = 0; i < instanceCount; i++)
        {
            GameData data;
            data.setNickname("Manager" + std::to_string(i));
            std::array<std::size_t, 4> scores = {2000 + i, 3000 + i, 4000 + i, 5000 + i};
            data.setHighscores(scores);
            managers[i]->setGamedata(data);
            ASSERT_TRUE(managers[i]->saveGame());
        }

        // Check the final state
        {
            DataManager dm;
            dm.init(m_testFilename);
            ASSERT_EQ(dm.getGamedata().getNickname(), "Manager" + std::to_string(instanceCount - 1));
            std::array<std::size_t, 4> expectedScores = {
                2000 + instanceCount - 1,
                3000 + instanceCount - 1,
                4000 + instanceCount - 1,
                5000 + instanceCount - 1};
            ASSERT_EQ(dm.getGamedata().getHighscores(), expectedScores);
        }

        // Release all managers
        managers.clear();

        // Verify file access still works
        {
            DataManager dm;
            dm.init(m_testFilename);
            const GameData &data = dm.getGamedata();
            ASSERT_FALSE(data.getNickname().empty());
        }
    }

} // namespace datacoe