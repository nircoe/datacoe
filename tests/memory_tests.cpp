#include <datacoe/save_load.hpp>
#include <support/test_support.hpp>
#include <gtest/gtest.h>
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <format>
#include <string>
#include <vector>

using namespace datacoe;
using namespace datacoe_test;

//==============================================================================
//                  MemoryTests - repeated and large data tests
//==============================================================================

class MemoryTests : public ::testing::Test
{
protected:
    temp_dir m_dir;
};

//==============================================================================
//                               Repeated Save and Load
//==============================================================================

TEST_F(MemoryTests, RepeatedSaveLoad)
{
    constexpr int iterations = 100;
    const auto path = m_dir / "repeated.json";

    // Test 1: save and load the same path over and over, the last value wins
    for (int i = 0; i < iterations; ++i)
    {
        const auto saved = save(path, player_save{"MemoryTest", i, {i}});
        ASSERT_TRUE(saved.has_value()) << saved.error().message;

        const auto loaded = load<player_save>(path);
        ASSERT_TRUE(loaded.has_value()) << loaded.error().message;
    }

    const auto final_load = load<player_save>(path);
    ASSERT_TRUE(final_load.has_value()) << final_load.error().message;
    EXPECT_EQ(*final_load, (player_save{"MemoryTest", iterations - 1, {iterations - 1}}));
}

//==============================================================================
//                                 Large Payload
//==============================================================================

TEST_F(MemoryTests, LargePayload)
{
    player_save data{std::string(1024 * 1024, 'A'), 999999, {}};
    data.unlocked.resize(100000);
    for (std::size_t i = 0; i < data.unlocked.size(); ++i)
        data.unlocked[i] = static_cast<int>(i);

    // Test 1: 1 MB nickname and 100k unlocked entries
    {
        const auto path = m_dir / "large.json";

        const auto saved = save(path, data);
        ASSERT_TRUE(saved.has_value()) << saved.error().message;

        const auto loaded = load<player_save>(path);
        ASSERT_TRUE(loaded.has_value()) << loaded.error().message;
        EXPECT_EQ(*loaded, data);
    }

    // Test 2: Encrypted, the same payload
    {
        const auto path = m_dir / "large.sav";

        const auto saved = save(path, data, true);
        ASSERT_TRUE(saved.has_value()) << saved.error().message;
        EXPECT_TRUE(read_text(path).starts_with("DATACOE_ENCRYPTED"));

        const auto loaded = load<player_save>(path);
        ASSERT_TRUE(loaded.has_value()) << loaded.error().message;
        EXPECT_EQ(*loaded, data);
    }
}

//==============================================================================
//                                  Many Files
//==============================================================================

TEST_F(MemoryTests, ManyFiles)
{
    constexpr int file_count = 100;
    std::vector<std::string> expected_names;

    // Test 1: every file holds its own data
    for (int i = 0; i < file_count; ++i)
    {
        const auto name = std::format("slot_{}.json", i);
        expected_names.push_back(name);

        const auto saved = save(m_dir / name, player_save{std::format("player_{}", i), i, {i, i + 1}});
        ASSERT_TRUE(saved.has_value()) << saved.error().message;
    }

    for (int i = 0; i < file_count; ++i)
    {
        const auto loaded = load<player_save>(m_dir / expected_names[static_cast<std::size_t>(i)]);
        ASSERT_TRUE(loaded.has_value()) << loaded.error().message;
        EXPECT_EQ(*loaded, (player_save{std::format("player_{}", i), i, {i, i + 1}}));
    }

    // Test 2: the directory holds exactly those files, no .tmp leftovers
    std::vector<std::string> found_names;
    for (const auto &entry : std::filesystem::directory_iterator(m_dir.path()))
        found_names.push_back(entry.path().filename().string());

    std::ranges::sort(expected_names);
    std::ranges::sort(found_names);
    EXPECT_EQ(found_names, expected_names);
}
