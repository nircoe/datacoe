#include "gtest/gtest.h"
#include "data_manager.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>

namespace DataManagement {

class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_testFilename = "perf_test_data.json";
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

    template<typename Func>
    long long measureExecutionTime(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    std::string m_testFilename;
};

TEST_F(PerformanceTest, SavePerformance) {
    constexpr int iterations = 100;
    
    DataManager dm;
    dm.init(m_testFilename);
    dm.setNickName("PerformanceTest");
    dm.setHighScore(10000);
    
    std::vector<long long> timings;
    timings.reserve(iterations);
    
    // Measure save performance
    for (int i = 0; i < iterations; i++) {
        dm.setHighScore(10000 + i); // Change data slightly each iteration
        auto duration = measureExecutionTime([&]() {
            dm.saveGame();
        });
        timings.push_back(duration);
    }
    
    // Calculate statistics
    double avg = 0.0;
    for (auto time : timings) {
        avg += time;
    }
    avg /= iterations;
    
    std::sort(timings.begin(), timings.end());
    long long median = timings[iterations / 2];
    long long p95 = timings[static_cast<int>(iterations * 0.95)];
    long long min = timings.front();
    long long max = timings.back();
    
    std::cout << "Save Performance (microseconds):" << std::endl;
    std::cout << "  Average: " << avg << std::endl;
    std::cout << "  Median: " << median << std::endl;
    std::cout << "  95th percentile: " << p95 << std::endl;
    std::cout << "  Min: " << min << std::endl;
    std::cout << "  Max: " << max << std::endl;
    
    // No strict assertions here, just reporting performance metrics
    // You might want to add baseline assertions based on your performance requirements
    // For example:
    // ASSERT_LT(avg, 10000) << "Average save time exceeds 10ms";
}

TEST_F(PerformanceTest, LoadPerformance) {
    constexpr int iterations = 100;
    
    // First create a file to load
    {
        DataManager dm;
        dm.init(m_testFilename);
        dm.setNickName("PerformanceTest");
        dm.setHighScore(10000);
        dm.saveGame();
    }
    
    std::vector<long long> timings;
    timings.reserve(iterations);
    
    // Measure load performance
    for (int i = 0; i < iterations; i++) {
        auto duration = measureExecutionTime([&]() {
            DataManager dm;
            dm.init(m_testFilename);
            // Force load by accessing game data
            auto data = dm.getGameData();
        });
        timings.push_back(duration);
    }
    
    // Calculate statistics
    double avg = 0.0;
    for (auto time : timings) {
        avg += time;
    }
    avg /= iterations;
    
    std::sort(timings.begin(), timings.end());
    long long median = timings[iterations / 2];
    long long p95 = timings[static_cast<int>(iterations * 0.95)];
    long long min = timings.front();
    long long max = timings.back();
    
    std::cout << "Load Performance (microseconds):" << std::endl;
    std::cout << "  Average: " << avg << std::endl;
    std::cout << "  Median: " << median << std::endl;
    std::cout << "  95th percentile: " << p95 << std::endl;
    std::cout << "  Min: " << min << std::endl;
    std::cout << "  Max: " << max << std::endl;
    
    // No strict assertions here, just reporting performance metrics
}

TEST_F(PerformanceTest, StressTest) {
    constexpr int iterations = 500;
    
    DataManager dm;
    dm.init(m_testFilename);
    
    // Random generator for mixed operations
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> opDist(0, 2); // 0=save, 1=load, 2=new game
    std::uniform_int_distribution<> scoreDist(0, 100000);
    
    std::vector<std::string> names = {"Player1", "Player2", "Player3", "Gamer", "Pro", "Noob", "Champion"};
    std::uniform_int_distribution<> nameDist(0, static_cast<int>(names.size() - 1));
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        int operation = opDist(gen);
        
        switch (operation) {
            case 0: // Save
                dm.setNickName(names[nameDist(gen)]);
                dm.setHighScore(scoreDist(gen));
                dm.saveGame();
                break;
            case 1: // Load
                dm.loadGame();
                break;
            case 2: // New game
                dm.newGame();
                break;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "Stress Test Results:" << std::endl;
    std::cout << "  Total time for " << iterations << " operations: " << totalDuration << "ms" << std::endl;
    std::cout << "  Average time per operation: " << (totalDuration * 1000.0 / iterations) << "us" << std::endl;
    
    // Verify the manager is still functional after stress
    dm.setNickName("FinalCheck");
    dm.setHighScore(12345);
    ASSERT_TRUE(dm.saveGame());
    
    DataManager dm2;
    dm2.init(m_testFilename);
    ASSERT_EQ(dm2.getGameData().getNickName(), "FinalCheck");
    ASSERT_EQ(dm2.getGameData().getHighscore(), 12345);
}

} // namespace DataManagement