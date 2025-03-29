#include <gtest/gtest.h>
#include <datacoe/data_manager.hpp>
#include <datacoe/data_reader_writer.hpp>
#include <filesystem>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>

namespace datacoe
{
    class PerformanceTest : public ::testing::Test
    {
    protected:
        std::string m_testFilename;

        void SetUp() override
        {
            m_testFilename = "perf_test_data.json";
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

        template <typename Func>
        long long measureExecutionTime(Func &&func)
        {
            auto start = std::chrono::high_resolution_clock::now();
            func();
            auto end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        }
    };

    TEST_F(PerformanceTest, SavePerformance)
    {
        constexpr int iterations = 100;

        DataManager dm;
        bool initResult = dm.init(m_testFilename);
        ASSERT_FALSE(initResult) << "init() should return false for new file";

        GameData gameData;
        gameData.setNickname("PerformanceTest");
        gameData.setHighscore(10000);
        dm.setGamedata(gameData);

        std::vector<long long> timings;
        timings.reserve(iterations);

        // Measure save performance
        for (int i = 0; i < iterations; i++)
        {
            // Change data slightly each iteration
            GameData updatedData = dm.getGamedata();
            updatedData.setHighscore(10000 + i);
            dm.setGamedata(updatedData);

            auto duration = measureExecutionTime([&]()
                                                 { dm.saveGame(); });
            timings.push_back(duration);
        }

        // Calculate statistics
        double avg = 0.0;
        for (auto time : timings)
        {
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

    TEST_F(PerformanceTest, LoadPerformance)
    {
        constexpr int iterations = 100;

        // First create a file to load
        {
            DataManager dm;
            bool initResult = dm.init(m_testFilename);
            ASSERT_FALSE(initResult) << "init() should return false for new file";

            GameData gameData;
            gameData.setNickname("PerformanceTest");
            gameData.setHighscore(10000);
            dm.setGamedata(gameData);

            dm.saveGame();
        }

        std::vector<long long> timings;
        timings.reserve(iterations);

        // Measure load performance
        for (int i = 0; i < iterations; i++)
        {
            auto duration = measureExecutionTime([&]()
                                                 {
            DataManager dm;
            bool loadResult = dm.init(m_testFilename);
            ASSERT_TRUE(loadResult) << "init() should return true when loading existing file";
            // Force load by accessing game data
            auto data = dm.getGamedata(); });
            timings.push_back(duration);
        }

        // Calculate statistics
        double avg = 0.0;
        for (auto time : timings)
        {
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

    TEST_F(PerformanceTest, StressTest)
    {
        constexpr int iterations = 500;

        DataManager dm;
        bool initResult = dm.init(m_testFilename);
        ASSERT_FALSE(initResult) << "init() should return false for new file";

        // Random generator for mixed operations
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> opDist(0, 2); // 0=save, 1=load, 2=new game
        std::uniform_int_distribution<> scoreDist(0, 100000);

        std::vector<std::string> names = {"Player1", "Player2", "Player3", "Gamer", "Pro", "Noob", "Champion"};
        std::uniform_int_distribution<> nameDist(0, static_cast<int>(names.size() - 1));

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < iterations; i++)
        {
            int operation = opDist(gen);

            switch (operation)
            {
            case 0: // Save
            {
                GameData gameData;
                gameData.setNickname(names[nameDist(gen)]);
                gameData.setHighscore(scoreDist(gen));
                dm.setGamedata(gameData);
                dm.saveGame();
                break;
            }
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
        GameData finalData;
        finalData.setNickname("FinalCheck");
        finalData.setHighscore(12345);
        dm.setGamedata(finalData);
        ASSERT_TRUE(dm.saveGame());

        DataManager dm2;
        bool finalLoadResult = dm2.init(m_testFilename);
        ASSERT_TRUE(finalLoadResult) << "init() should return true after saving final data";
        ASSERT_EQ(dm2.getGamedata().getNickname(), "FinalCheck");
        ASSERT_EQ(dm2.getGamedata().getHighscore(), 12345);
    }

    TEST_F(PerformanceTest, EncryptionPerformanceComparison)
    {
        constexpr int iterations = 50;

        // Setup test data
        GameData testData("PerformanceTest", 12345);
        std::string encryptedFilename = m_testFilename + ".encrypted";
        std::string unencryptedFilename = m_testFilename + ".unencrypted";

        std::vector<long long> encryptedSaveTimings;
        std::vector<long long> unencryptedSaveTimings;
        std::vector<long long> encryptedLoadTimings;
        std::vector<long long> unencryptedLoadTimings;

        try
        {
            encryptedSaveTimings.reserve(iterations);
            unencryptedSaveTimings.reserve(iterations);
            encryptedLoadTimings.reserve(iterations);
            unencryptedLoadTimings.reserve(iterations);

            // Create initial files
            ASSERT_TRUE(DataReaderWriter::writeData(testData, encryptedFilename, true));
            ASSERT_TRUE(DataReaderWriter::writeData(testData, unencryptedFilename, false));

            // Measure save performance
            for (int i = 0; i < iterations; i++)
            {
                // Modify data slightly to avoid caching effects
                testData.setHighscore(12345 + i);

                // Measure encrypted save
                auto encryptedSaveTime = measureExecutionTime([&]()
                                                              { DataReaderWriter::writeData(testData, encryptedFilename, true); });
                encryptedSaveTimings.push_back(encryptedSaveTime);

                // Measure unencrypted save
                auto unencryptedSaveTime = measureExecutionTime([&]()
                                                                { DataReaderWriter::writeData(testData, unencryptedFilename, false); });
                unencryptedSaveTimings.push_back(unencryptedSaveTime);
            }

            // Measure load performance
            for (int i = 0; i < iterations; i++)
            {
                // Measure encrypted load
                auto encryptedLoadTime = measureExecutionTime([&]()
                                                              { auto data = DataReaderWriter::readData(encryptedFilename, true); });
                encryptedLoadTimings.push_back(encryptedLoadTime);

                // Measure unencrypted load
                auto unencryptedLoadTime = measureExecutionTime([&]()
                                                                { auto data = DataReaderWriter::readData(unencryptedFilename, false); });
                unencryptedLoadTimings.push_back(unencryptedLoadTime);
            }

            // Calculate statistics for encrypted save
            double encSaveAvg = 0.0;
            for (auto time : encryptedSaveTimings)
            {
                encSaveAvg += time;
            }
            encSaveAvg /= iterations;

            // Calculate statistics for unencrypted save
            double unencSaveAvg = 0.0;
            for (auto time : unencryptedSaveTimings)
            {
                unencSaveAvg += time;
            }
            unencSaveAvg /= iterations;

            // Calculate statistics for encrypted load
            double encLoadAvg = 0.0;
            for (auto time : encryptedLoadTimings)
            {
                encLoadAvg += time;
            }
            encLoadAvg /= iterations;

            // Calculate statistics for unencrypted load
            double unencLoadAvg = 0.0;
            for (auto time : unencryptedLoadTimings)
            {
                unencLoadAvg += time;
            }
            unencLoadAvg /= iterations;

            // Calculate performance impact percentages
            double saveImpact = ((encSaveAvg / unencSaveAvg) - 1.0) * 100.0;
            double loadImpact = ((encLoadAvg / unencLoadAvg) - 1.0) * 100.0;

            // Output results
            std::cout << "=============================================" << std::endl;
            std::cout << "     Encryption Performance Comparison" << std::endl;
            std::cout << "=============================================" << std::endl;
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Save operations (microseconds):" << std::endl;
            std::cout << "  Encrypted average: " << encSaveAvg << std::endl;
            std::cout << "  Unencrypted average: " << unencSaveAvg << std::endl;
            std::cout << "  Encryption overhead: " << (saveImpact > 0 ? "+" : "") << saveImpact << "%" << std::endl;
            std::cout << std::endl;

            std::cout << "Load operations (microseconds):" << std::endl;
            std::cout << "  Encrypted average: " << encLoadAvg << std::endl;
            std::cout << "  Unencrypted average: " << unencLoadAvg << std::endl;
            std::cout << "  Encryption overhead: " << (loadImpact > 0 ? "+" : "") << loadImpact << "%" << std::endl;
            std::cout << "=============================================" << std::endl;

            // Also measure file size difference
            std::uintmax_t encryptedSize = std::filesystem::file_size(encryptedFilename);
            std::uintmax_t unencryptedSize = std::filesystem::file_size(unencryptedFilename);
            double sizeImpact = ((double)encryptedSize / unencryptedSize - 1.0) * 100.0;

            std::cout << "File size comparison:" << std::endl;
            std::cout << "  Encrypted: " << encryptedSize << " bytes" << std::endl;
            std::cout << "  Unencrypted: " << unencryptedSize << " bytes" << std::endl;
            std::cout << "  Size overhead: " << (sizeImpact > 0 ? "+" : "") << sizeImpact << "%" << std::endl;
            std::cout << "=============================================" << std::endl;
        }
        catch (const std::exception &e)
        {
            // Clean up in exception path
            try
            {
                if (std::filesystem::exists(encryptedFilename))
                    std::filesystem::remove(encryptedFilename);
                if (std::filesystem::exists(unencryptedFilename))
                    std::filesystem::remove(unencryptedFilename);
            }
            catch (...)
            { /* Ignore cleanup errors */
            }

            FAIL() << "Unexpected exception: " << e.what();
        }

        // Clean up in normal path
        try
        {
            if (std::filesystem::exists(encryptedFilename))
                std::filesystem::remove(encryptedFilename);
            if (std::filesystem::exists(unencryptedFilename))
                std::filesystem::remove(unencryptedFilename);
        }
        catch (...)
        { /* Ignore cleanup errors */
        }
    }
} // namespace datacoe