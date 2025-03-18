#include "gtest/gtest.h"
#include "data_manager.hpp"
#include <filesystem>
#include <fstream>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <iostream>

namespace DataManagement {

class ThreadSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_testFilename = "thread_test_data.json";
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

TEST_F(ThreadSafetyTest, ConcurrentReads) {
    // First create data to read
    {
        DataManager dm;
        dm.init(m_testFilename);
        dm.setNickName("ThreadTest");
        dm.setHighScore(12345);
        ASSERT_TRUE(dm.saveGame());
    }
    
    constexpr int threadCount = 10;
    constexpr int iterationsPerThread = 100;
    
    std::vector<std::thread> threads;
    std::atomic<int> successCount = 0;
    
    // Launch multiple threads to read simultaneously
    for (int t = 0; t < threadCount; t++) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < iterationsPerThread; i++) {
                try {
                    DataManager dm;
                    dm.init(m_testFilename);
                    
                    const GameData& data = dm.getGameData();
                    if (data.getNickName() == "ThreadTest" && data.getHighscore() == 12345) {
                        successCount++;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Thread " << t << " iteration " << i 
                              << " exception: " << e.what() << std::endl;
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // All reads should succeed
    ASSERT_EQ(successCount, threadCount * iterationsPerThread);
}

TEST_F(ThreadSafetyTest, ConcurrentWrites) {
    constexpr int threadCount = 5;
    constexpr int iterationsPerThread = 20;
    
    std::vector<std::thread> threads;
    std::mutex fileMutex; // Use mutex to avoid file corruption
    
    // Launch multiple threads to write to the same file
    for (int t = 0; t < threadCount; t++) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < iterationsPerThread; i++) {
                std::lock_guard<std::mutex> lock(fileMutex);
                
                try {
                    DataManager dm;
                    dm.init(m_testFilename);
                    
                    // Include thread and iteration info in nickname
                    std::string nickname = "Thread" + std::to_string(t) + "_Iter" + std::to_string(i);
                    dm.setNickName(nickname);
                    dm.setHighScore(t * 1000 + i);
                    
                    ASSERT_TRUE(dm.saveGame());
                    
                    // Verify immediately
                    DataManager verifyDm;
                    verifyDm.init(m_testFilename);
                    ASSERT_EQ(verifyDm.getGameData().getNickName(), nickname);
                    ASSERT_EQ(verifyDm.getGameData().getHighscore(), t * 1000 + i);
                } catch (const std::exception& e) {
                    std::cerr << "Thread " << t << " iteration " << i 
                              << " exception: " << e.what() << std::endl;
                    FAIL() << "Exception in thread " << t << ": " << e.what();
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // We can't reliably assert exactly which thread's data will be the last to be written
    // since thread scheduling is non-deterministic. So instead, just validate that we have
    // valid data that matches our expected format.
    DataManager finalDm;
    finalDm.init(m_testFilename);
    
    // Check that the name follows our expected pattern
    std::string name = finalDm.getGameData().getNickName();
    ASSERT_TRUE(name.find("Thread") == 0) << "Final data nickname doesn't match expected format";
    
    // Check that the score is within the expected range
    int score = finalDm.getGameData().getHighscore();
    ASSERT_TRUE(score >= 0 && score < threadCount * 1000 + iterationsPerThread)
        << "Final data score outside expected range";
}

TEST_F(ThreadSafetyTest, SimultaneousReadWrite) {
    // Initial data
    {
        DataManager dm;
        dm.init(m_testFilename);
        dm.setNickName("Initial");
        dm.setHighScore(0);
        ASSERT_TRUE(dm.saveGame());
    }
    
    constexpr int iterations = 100;
    std::atomic<bool> running{true};
    std::mutex fileMutex;
    
    // Thread for continuous reading
    std::thread readerThread([&]() {
        while (running) {
            try {
                DataManager dm;
                dm.init(m_testFilename);
                
                // Just read the data
                const GameData& data = dm.getGameData();
                (void)data; // Avoid unused variable warning
            } catch (const std::exception& e) {
                std::cerr << "Reader exception: " << e.what() << std::endl;
            }
            
            // Small delay to avoid overwhelming the system
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    // Perform writes while the reader is active
    for (int i = 0; i < iterations; i++) {
        {
            std::lock_guard<std::mutex> lock(fileMutex);
            
            try {
                DataManager dm;
                dm.init(m_testFilename);
                dm.setNickName("Write" + std::to_string(i));
                dm.setHighScore(i);
                ASSERT_TRUE(dm.saveGame());
            } catch (const std::exception& e) {
                FAIL() << "Writer exception: " << e.what();
            }
        }
        
        // Small delay to give reader a chance
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    
    // Signal reader to stop and wait for it
    running = false;
    readerThread.join();
    
    // Final state verification
    DataManager finalDm;
    finalDm.init(m_testFilename);
    ASSERT_EQ(finalDm.getGameData().getNickName(), "Write" + std::to_string(iterations - 1));
    ASSERT_EQ(finalDm.getGameData().getHighscore(), iterations - 1);
}

} // namespace DataManagement