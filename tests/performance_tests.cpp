#include <datacoe/save_load.hpp>
#include <datacoe_config.hpp>
#include <support/test_support.hpp>
#include <gtest/gtest.h>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <system_error>
#include <vector>

using namespace datacoe;
using namespace datacoe_test;

namespace
{
    template <typename Func>
    long long measure_microseconds(Func &&func)
    {
        const auto start = std::chrono::steady_clock::now();
        func();
        const auto end = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    long long timed_save(const std::filesystem::path &path, const player_save &data, bool encrypt = false)
    {
        std::expected<void, error> saved;
        const auto time = measure_microseconds([&] { saved = save(path, data, encrypt); });
        EXPECT_TRUE(saved.has_value()) << saved.error().message;
        return time;
    }

    long long timed_load(const std::filesystem::path &path, const player_save &expected)
    {
        std::expected<player_save, error> loaded;
        const auto time = measure_microseconds([&] { loaded = load<player_save>(path); });
        EXPECT_TRUE(loaded.has_value()) << loaded.error().message;
        if (loaded.has_value())
        {
            EXPECT_EQ(*loaded, expected);
        }
        return time;
    }

    double average(const std::vector<long long> &timings)
    {
        return static_cast<double>(std::accumulate(timings.begin(), timings.end(), 0LL)) /
               static_cast<double>(timings.size());
    }

    void print_stats(const std::string &title, std::vector<long long> timings)
    {
        std::ranges::sort(timings);

        std::cout << title << " (microseconds):" << std::endl;
        std::cout << "  Average: " << average(timings) << std::endl;
        std::cout << "  Median: " << timings[timings.size() / 2] << std::endl;
        std::cout << "  95th percentile: " << timings[timings.size() * 95 / 100] << std::endl;
        std::cout << "  Min: " << timings.front() << std::endl;
        std::cout << "  Max: " << timings.back() << std::endl;
    }

    double overhead_percent(double measured, double baseline)
    {
        return (measured / baseline - 1.0) * 100.0;
    }
} // namespace

//==============================================================================
//                  PerformanceTests - save and load benchmarks
//==============================================================================

class PerformanceTests : public ::testing::Test
{
protected:
    temp_dir m_dir;
};

//==============================================================================
//                            Save and Load Timing
//==============================================================================

TEST_F(PerformanceTests, SavePerformance)
{
    constexpr int iterations = 100;
    const auto path = m_dir / "perf.json";
    std::vector<long long> timings;

    // Test 1: save with slightly different data each time
    player_save data{"PerformanceTest", 10000, {}};
    for (int i = 0; i < iterations; ++i)
    {
        data.highscore = 10000 + i;
        timings.push_back(timed_save(path, data));
    }

    print_stats("Save Performance", timings);

    // Test 2: the last save is what's on disk
    const auto loaded = load<player_save>(path);
    ASSERT_TRUE(loaded.has_value()) << loaded.error().message;
    EXPECT_EQ(*loaded, data);
}

TEST_F(PerformanceTests, LoadPerformance)
{
    constexpr int iterations = 100;
    const auto path = m_dir / "perf.json";
    const player_save data{"PerformanceTest", 10000, {}};
    std::vector<long long> timings;

    // Test 1: load the same file over and over
    const auto saved = save(path, data);
    ASSERT_TRUE(saved.has_value()) << saved.error().message;

    for (int i = 0; i < iterations; ++i)
        timings.push_back(timed_load(path, data));

    print_stats("Load Performance", timings);
}

//==============================================================================
//                                 Stress Test
//==============================================================================

TEST_F(PerformanceTests, StressTest)
{
    constexpr int iterations = 500;
    const auto path = m_dir / "stress.json";
    const std::vector<std::string> names = {"Player1", "Player2", "Player3", "Gamer", "Pro", "Noob", "Champion"};

    std::mt19937 gen(42);
    std::uniform_int_distribution<> operation_dist(0, 2); // 0=save, 1=load, 2=new game
    std::uniform_int_distribution<> score_dist(0, 100000);
    std::uniform_int_distribution<> name_dist(0, static_cast<int>(names.size()) - 1);

    // Test 1: random mix of saves, loads and new games, every load matches the last save
    player_save last_saved;
    {
        const auto saved = save(path, last_saved);
        ASSERT_TRUE(saved.has_value()) << saved.error().message;
    }

    const auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < iterations; ++i)
    {
        const int operation = operation_dist(gen);

        if (operation == 1)
        {
            const auto loaded = load<player_save>(path);
            ASSERT_TRUE(loaded.has_value()) << loaded.error().message;
            EXPECT_EQ(*loaded, last_saved);
            continue;
        }

        last_saved = operation == 0 ? player_save{names[static_cast<std::size_t>(name_dist(gen))], score_dist(gen), {}}
                                    : player_save{};
        const auto saved = save(path, last_saved);
        ASSERT_TRUE(saved.has_value()) << saved.error().message;
    }

    const auto total_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

    std::cout << "Stress Test Results:" << std::endl;
    std::cout << "  Total time for " << iterations << " operations: " << total_ms << "ms" << std::endl;
    std::cout << "  Average time per operation: " << (static_cast<double>(total_ms) * 1000.0 / iterations) << "us"
              << std::endl;

    // Test 2: still works after the stress
    {
        const player_save final_data{"FinalCheck", 12345, {}};
        timed_save(path, final_data);
        timed_load(path, final_data);
    }
}

//==============================================================================
//                                  Encryption
//==============================================================================

TEST_F(PerformanceTests, EncryptedPerformanceComparison)
{
    if constexpr (!DATACOE_ENABLE_ENCRYPTION)
        GTEST_SKIP() << "needs DATACOE_ENABLE_ENCRYPTION";

    constexpr int iterations = 50;
    const auto encrypted_path = m_dir / "perf.sav";
    const auto plain_path = m_dir / "perf.json";

    std::vector<long long> encrypted_saves;
    std::vector<long long> plain_saves;
    std::vector<long long> encrypted_loads;
    std::vector<long long> plain_loads;

    // Test 1: save and load the same data both ways
    player_save data{"PerformanceTest", 12345, {}};
    for (int i = 0; i < iterations; ++i)
    {
        data.highscore = 12345 + i;
        encrypted_saves.push_back(timed_save(encrypted_path, data, true));
        plain_saves.push_back(timed_save(plain_path, data));
    }

    for (int i = 0; i < iterations; ++i)
    {
        encrypted_loads.push_back(timed_load(encrypted_path, data));
        plain_loads.push_back(timed_load(plain_path, data));
    }

    // Test 2: only the encrypted file has the prefix
    EXPECT_TRUE(read_text(encrypted_path).starts_with(ENCRYPTED_PREFIX));
    EXPECT_FALSE(read_text(plain_path).starts_with(ENCRYPTED_PREFIX));

    const double save_overhead = overhead_percent(average(encrypted_saves), average(plain_saves));
    const double load_overhead = overhead_percent(average(encrypted_loads), average(plain_loads));

    std::error_code ec;
    const auto encrypted_size = static_cast<double>(std::filesystem::file_size(encrypted_path, ec));
    const auto plain_size = static_cast<double>(std::filesystem::file_size(plain_path, ec));

    std::cout << "=============================================" << std::endl;
    std::cout << "     Encryption Performance Comparison" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Save operations (microseconds):" << std::endl;
    std::cout << "  Encrypted average: " << average(encrypted_saves) << std::endl;
    std::cout << "  Plain average: " << average(plain_saves) << std::endl;
    std::cout << "  Encryption overhead: " << save_overhead << "%" << std::endl;
    std::cout << "Load operations (microseconds):" << std::endl;
    std::cout << "  Encrypted average: " << average(encrypted_loads) << std::endl;
    std::cout << "  Plain average: " << average(plain_loads) << std::endl;
    std::cout << "  Encryption overhead: " << load_overhead << "%" << std::endl;
    std::cout << "File size (bytes):" << std::endl;
    std::cout << "  Encrypted: " << encrypted_size << std::endl;
    std::cout << "  Plain: " << plain_size << std::endl;
    std::cout << "  Size overhead: " << overhead_percent(encrypted_size, plain_size) << "%" << std::endl;
    std::cout << "=============================================" << std::endl;
}
