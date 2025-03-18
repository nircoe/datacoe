#include "gtest/gtest.h"
#include "data_manager.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace DataManagement {

class ErrorHandlingTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_testFilename = "error_test_data.json";
        m_corruptFilename = "corrupt_test_data.json";
        // Clean up any leftover files from previous tests
        try {
            if (std::filesystem::exists(m_testFilename)) {
                std::filesystem::remove(m_testFilename);
            }
            if (std::filesystem::exists(m_corruptFilename)) {
                std::filesystem::remove(m_corruptFilename);
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
            if (std::filesystem::exists(m_corruptFilename)) {
                std::filesystem::remove(m_corruptFilename);
            }
        } catch (const std::filesystem::filesystem_error&) {
            // Ignore errors
        }
    }

    std::string m_testFilename;
    std::string m_corruptFilename;

    // Helper to create a corrupted file
    void createCorruptJsonFile() {
        // First create a valid file
        {
            DataManager dm;
            dm.init(m_corruptFilename);
            dm.setNickName("ValidData");
            dm.setHighScore(500);
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
    bool createReadOnlyFile() {
#ifdef _WIN32
        // Create file
        {
            DataManager dm;
            dm.init(m_testFilename);
            dm.setNickName("ReadOnly");
            dm.setHighScore(100);
            bool saveResult = dm.saveGame();
            if (!saveResult) {
                return false;
            }
        }
        
        // Make it read-only
        std::string command = "attrib +r " + m_testFilename;
        if (system(command.c_str()) != 0) {
            return false;
        }
        return true;
#else
        // Create file
        {
            DataManager dm;
            dm.init(m_testFilename);
            dm.setNickName("ReadOnly");
            dm.setHighScore(100);
            bool saveResult = dm.saveGame();
            if (!saveResult) {
                return false;
            }
        }
        
        // Make it read-only
        if (chmod(m_testFilename.c_str(), S_IRUSR | S_IRGRP | S_IROTH) != 0) {
            return false;
        }
        return true;
#endif
    }

    // Helper to restore writability
    void restoreWritePermission() {
#ifdef _WIN32
        std::string command = "attrib -r " + m_testFilename;
        system(command.c_str());
#else
        chmod(m_testFilename.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif
    }
};

TEST_F(ErrorHandlingTest, CorruptJsonFile) {
    createCorruptJsonFile();
    
    // Attempt to load corrupt data
    DataManager dm;
    dm.init(m_corruptFilename);
    
    // Should either initialize with defaults or throw an exception
    // Either way, the manager should be in a valid state
    
    // Just check that we can still use the manager
    dm.setNickName("RecoveredData");
    dm.setHighScore(999);
    bool saveResult = dm.saveGame();
    ASSERT_TRUE(saveResult) << "Failed to save after recovery";
    
    // Try loading again - should work now
    DataManager dm2;
    dm2.init(m_corruptFilename);
    ASSERT_EQ(dm2.getGameData().getNickName(), "RecoveredData");
    ASSERT_EQ(dm2.getGameData().getHighscore(), 999);
}

TEST_F(ErrorHandlingTest, NonExistentDirectory) {
    std::string nonExistentPath = "non/existent/directory/file.json";
    
    DataManager dm;
    dm.init(nonExistentPath);
    
    // Should still be able to set data
    dm.setNickName("TestNonExistent");
    dm.setHighScore(123);
    
    // Save will likely fail but shouldn't crash
    bool saveResult = dm.saveGame();
    
    // We don't assert on saveResult because it depends on implementation
    // The key is that the call doesn't crash
    
    std::cout << "Save to non-existent directory " 
              << (saveResult ? "succeeded" : "failed as expected") << std::endl;
}

TEST_F(ErrorHandlingTest, EmptyFilename) {
    DataManager dm;
    dm.init(""); // Empty filename
    
    // Should still be able to use the manager
    dm.setNickName("EmptyFilename");
    dm.setHighScore(123);
    
    // Save may fail but shouldn't crash
    bool saveResult = dm.saveGame();
    std::cout << "Save with empty filename " 
              << (saveResult ? "succeeded" : "failed as expected") << std::endl;
}

TEST_F(ErrorHandlingTest, ReadOnlyFile) {
    // Skip if we can't make a read-only file
    if (!createReadOnlyFile()) {
        GTEST_SKIP() << "Cannot create read-only file for testing";
    }
    
    // Try to save to a read-only file
    DataManager dm;
    dm.init(m_testFilename);
    dm.setNickName("NewData");
    dm.setHighScore(200);
    
    // Save will likely fail but shouldn't crash
    bool saveResult = dm.saveGame();
    std::cout << "Save to read-only file " 
              << (saveResult ? "succeeded unexpectedly" : "failed as expected") << std::endl;
    
    // Restore write permission for cleanup
    restoreWritePermission();
}

TEST_F(ErrorHandlingTest, InterruptedSave) {
    // Test handling of partial/interrupted saves
    
    // First create valid data
    {
        DataManager dm;
        dm.init(m_testFilename);
        dm.setNickName("Original");
        dm.setHighScore(100);
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
    dm.init(m_testFilename);
    
    // Manager should still be usable
    dm.setNickName("Recovered");
    dm.setHighScore(200);
    bool saveResult = dm.saveGame();
    ASSERT_TRUE(saveResult) << "Failed to save after recovery";
    
    // Verify recovery worked
    DataManager dm2;
    dm2.init(m_testFilename);
    ASSERT_EQ(dm2.getGameData().getNickName(), "Recovered");
    ASSERT_EQ(dm2.getGameData().getHighscore(), 200);
}

TEST_F(ErrorHandlingTest, MalformedJson) {
    // Create a file with valid JSON but wrong structure
    {
        std::ofstream file(m_testFilename);
        file << R"({"wrongKey": "wrongValue"})";
        file.close();
    }
    
    // Try to load the malformed file
    DataManager dm;
    dm.init(m_testFilename);
    
    // Verify we can save valid data
    dm.setNickName("FixedData");
    dm.setHighScore(300);
    bool saveResult = dm.saveGame();
    ASSERT_TRUE(saveResult) << "Failed to save after malformed JSON recovery";
    
    // Check that the data was saved correctly
    DataManager dm2;
    dm2.init(m_testFilename);
    ASSERT_EQ(dm2.getGameData().getNickName(), "FixedData");
    ASSERT_EQ(dm2.getGameData().getHighscore(), 300);
}

} // namespace DataManagement