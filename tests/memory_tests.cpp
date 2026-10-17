#include <datacoe/save_load.hpp>
#include <datacoe_config.hpp>
#include <support/test_support.hpp>
#include <gtest/gtest.h>
#include <filesystem>
#include <format>
#include <iterator>
#include <numeric>
#include <string>

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
        const player_save expected{"MemoryTest", i, {i}};
        const auto saved = save(path, expected);
        ASSERT_TRUE(saved.has_value()) << saved.error().message;

        const auto loaded = load<player_save>(path);
        ASSERT_TRUE(loaded.has_value()) << loaded.error().message;
        EXPECT_EQ(*loaded, expected);
    }
}

//==============================================================================
//                                 Large Payload
//==============================================================================

TEST_F(MemoryTests, LargePayload)
{
    player_save data{std::string(1024 * 1024, 'A'), 999999, {}};
    data.unlocked.resize(100000);
    std::iota(data.unlocked.begin(), data.unlocked.end(), 0);

    // Test 1: 1 MB nickname and 100k unlocked entries
    {
        const auto path = m_dir / "large.json";

        const auto saved = save(path, data);
        ASSERT_TRUE(saved.has_value()) << saved.error().message;

        const auto loaded = load<player_save>(path);
        ASSERT_TRUE(loaded.has_value()) << loaded.error().message;
        EXPECT_EQ(*loaded, data);
    }
}

TEST_F(MemoryTests, LargeEncryptedPayload)
{
    if constexpr (!DATACOE_ENABLE_ENCRYPTION)
        GTEST_SKIP() << "needs DATACOE_ENABLE_ENCRYPTION";

    player_save data{std::string(1024 * 1024, 'A'), 999999, {}};
    data.unlocked.resize(100000);
    std::iota(data.unlocked.begin(), data.unlocked.end(), 0);

    // Test 1: the same payload
    {
        const auto path = m_dir / "large.sav";

        const auto saved = save(path, data, true);
        ASSERT_TRUE(saved.has_value()) << saved.error().message;
        EXPECT_TRUE(read_text(path).starts_with(ENCRYPTED_PREFIX));

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

    // Test 1: every file holds its own data
    for (int i = 0; i < file_count; ++i)
    {
        const auto saved =
            save(m_dir / std::format("slot_{}.json", i), player_save{std::format("player_{}", i), i, {i, i + 1}});
        ASSERT_TRUE(saved.has_value()) << saved.error().message;
    }

    for (int i = 0; i < file_count; ++i)
    {
        const auto loaded = load<player_save>(m_dir / std::format("slot_{}.json", i));
        ASSERT_TRUE(loaded.has_value()) << loaded.error().message;
        EXPECT_EQ(*loaded, (player_save{std::format("player_{}", i), i, {i, i + 1}}));
    }

    // Test 2: the directory holds exactly those files, a leftover .datacoe.tmp would add one
    const auto entries = std::distance(std::filesystem::directory_iterator(m_dir.path()),
                                       std::filesystem::directory_iterator());
    EXPECT_EQ(entries, file_count);
}
