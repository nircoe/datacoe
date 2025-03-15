#include "gtest/gtest.h"
#include "data_manager.hpp"
#include "data_reader_writer.hpp"
#include "game_data.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <iostream>

namespace DataManagement {

class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        testFilename = "test_performance.json";
        // Clean up any leftover files
        try {
            if (std::filesystem::exists(testFilename)) {
                std::filesystem::remove(testFilename);
            }
        } catch (const std::filesystem::filesystem_error&) {
            // Ignore if file doesn't exist
        }
    }

    void TearDown() override {
        // Give file handles time to close
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Try to clean up the test file
        for (int i = 0; i < 5; i++) {
            try {
                if (std::filesystem::exists(testFilename)) {
                    std::filesystem::remove(testFilename);
                }
                break;
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "TearDown() Error: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        // Clean up any batch test files
        for (int i = 0; i < 100; i++) {
            std::string batchFile = "test_batch_" + std::to_string(i) + ".json";
            try {
                if (std::filesystem::exists(batchFile)) {
                    std::filesystem::remove(batchFile);
                }
            } catch (...) {
                // Ignore errors
            }
        }
    }

    // Helper to measure execution time
    template<typename Func>
    long long measureExecutionTime(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    std::string testFilename;
};

TEST_F(PerformanceTest, ReadWritePerformance) {
    // Test basic read/write performance
    GameData testData("PerformanceTest", 1000);
    
    // Measure write time
    long long writeTime = measureExecutionTime([&]() {
        ASSERT_TRUE(DataReaderWriter::writeData(testData, testFilename));
    });
    
    std::cout << "Write time: " << writeTime << " microseconds" << std::endl;
    
    // Measure read time
    long long readTime = measureExecutionTime([&]() {
        std::optional<GameData> loaded = DataReaderWriter::readData(testFilename);
        ASSERT_TRUE(loaded.has_value());
    });
    
    std::cout << "Read time: " << readTime << " microseconds" << std::endl;
    
    // No specific assertions for timing, as it depends on the system
    // But we output the times for manual verification
}

TEST_F(PerformanceTest, LargeDataPerformance) {
    // Test with larger data
    std::string largeNickname(10000, 'X');  // 10KB nickname
    GameData largeData(largeNickname, 1000);
    
    // Measure write time for large data
    long long writeTime = measureExecutionTime([&]() {
        ASSERT_TRUE(DataReaderWriter::writeData(largeData, testFilename));
    });
    
    std::cout << "Large data write time: " << writeTime << " microseconds" << std::endl;
    
    // Measure read time for large data
    long long readTime = measureExecutionTime([&]() {
        std::optional<GameData> loaded = DataReaderWriter::readData(testFilename);
        ASSERT_TRUE(loaded.has_value());
        ASSERT_EQ(loaded.value().getNickName().length(), largeNickname.length());
    });
    
    std::cout << "Large data read time: " << readTime << " microseconds" << std::endl;
}

} // namespace DataManagement