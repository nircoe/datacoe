#include "gtest/gtest.h"
#include "data_manager.hpp"
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

namespace DataManagement {

class MemoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_testFilename = "memory_test_data.json";
        // Clean up any leftover files from previous tests
        try {
            if (std::filesystem::exists(m_testFilename)) {
                std::filesystem::remove(m_testFilename);
            }
        } catch (const std::filesystem::filesystem_error&) {
            // Ignore errors if file doesn't exist
        }
    }

    void TearDown() override {
        try {
            if (std::filesystem::exists(m_testFilename)) {
                std::filesystem::remove(m_testFilename);
            }
        } catch (const std::filesystem::filesystem_error&) {
            // Ignore errors
        }
    }

    std::string m_testFilename;
};

TEST_F(MemoryTest, RepeatedCreationAndDestruction) {
    // This test checks for memory leaks by repeatedly creating and destroying objects
    constexpr int iterations = 1000;
    
    for (int i = 0; i < iterations; i++) {
        DataManager dm;
        dm.init(m_testFilename);
        dm.setNickName("MemoryTest");
        dm.setHighScore(i);
        dm.saveGame();
    }
    
    // If there's a memory leak, this test will consume a lot of memory
    // Tools like Valgrind or Address Sanitizer should be used to detect actual leaks
    
    // Verify functionality still works
    DataManager finalDm;
    finalDm.init(m_testFilename);
    ASSERT_EQ(finalDm.getGameData().getNickName(), "MemoryTest");
    ASSERT_EQ(finalDm.getGameData().getHighscore(), iterations - 1);
}

TEST_F(MemoryTest, LargeDataHandling) {
    // Test with larger-than-typical data to check memory handling
    constexpr int dataSize = 1000; // Large enough to test memory usage but not too large
    
    // Create a DataManager with a large nickname
    {
        DataManager dm;
        dm.init(m_testFilename);
        
        // Generate a large string
        std::string largeString(dataSize, 'A');
        
        dm.setNickName(largeString);
        dm.setHighScore(999999);
        ASSERT_TRUE(dm.saveGame());
    }
    
    // Check data was saved correctly
    {
        DataManager dm;
        dm.init(m_testFilename);
        ASSERT_EQ(dm.getGameData().getNickName().size(), dataSize);
        ASSERT_EQ(dm.getGameData().getHighscore(), 999999);
    }
}

TEST_F(MemoryTest, MultipleInstancesWithSameFile) {
    // Test multiple DataManager instances using the same file
    constexpr int instanceCount = 10;
    
    // Create initial data
    {
        DataManager dm;
        dm.init(m_testFilename);
        dm.setNickName("InitialData");
        dm.setHighScore(1000);
        ASSERT_TRUE(dm.saveGame());
    }
    
    // Create multiple instances all pointing to the same file
    std::vector<std::unique_ptr<DataManager>> managers;
    for (int i = 0; i < instanceCount; i++) {
        auto dm = std::make_unique<DataManager>();
        dm->init(m_testFilename);
        managers.push_back(std::move(dm));
    }
    
    // Have each manager modify the data
    for (int i = 0; i < instanceCount; i++) {
        managers[i]->setNickName("Manager" + std::to_string(i));
        managers[i]->setHighScore(2000 + i);
        ASSERT_TRUE(managers[i]->saveGame());
    }
    
    // Check the final state
    {
        DataManager dm;
        dm.init(m_testFilename);
        ASSERT_EQ(dm.getGameData().getNickName(), "Manager" + std::to_string(instanceCount - 1));
        ASSERT_EQ(dm.getGameData().getHighscore(), 2000 + instanceCount - 1);
    }
    
    // Release all managers
    managers.clear();
    
    // Verify file access still works
    {
        DataManager dm;
        dm.init(m_testFilename);
        const GameData& data = dm.getGameData();
        ASSERT_FALSE(data.getNickName().empty());
    }
}

} // namespace DataManagement