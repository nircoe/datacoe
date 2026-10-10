#include <datacoe/save_load.hpp>
#include <support/test_support.hpp>
#include <gtest/gtest.h>
#include <expected>
#include <filesystem>

using namespace datacoe;
using namespace datacoe_test;

namespace
{
    template <typename T>
    void expect_error(const std::expected<T, error> &result, error_code code)
    {
        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error().code, code);
    }

    void expect_round_trip(const std::filesystem::path &path, const player_save &data, bool encrypt = false)
    {
        const auto saved = save(path, data, encrypt);
        ASSERT_TRUE(saved.has_value()) << saved.error().message;

        const auto loaded = load<player_save>(path);
        ASSERT_TRUE(loaded.has_value()) << loaded.error().message;
        EXPECT_EQ(*loaded, data);
    }

    void expect_loads(const std::filesystem::path &path, const player_save &expected)
    {
        const auto loaded = load<player_save>(path);
        ASSERT_TRUE(loaded.has_value()) << loaded.error().message;
        EXPECT_EQ(*loaded, expected);
    }
} // namespace

//==============================================================================
//                IntegrationTests - save and load workflow tests
//==============================================================================

class IntegrationTests : public ::testing::Test
{
protected:
    temp_dir m_dir;
};

//==============================================================================
//                                  Lifecycle
//==============================================================================

TEST_F(IntegrationTests, FullLifecycle)
{
    const auto path = m_dir / "game.sav";

    // Test 1: no save yet, this is the new game signal
    expect_error(load<player_save>(path), error_code::file_not_found);

    // Test 2: new game, save the default data
    player_save data;
    expect_round_trip(path, data);

    // Test 3: play, save again
    data.nickname = "Player1";
    data.highscore = 100;
    data.unlocked = {1, 2};
    expect_round_trip(path, data);

    // Test 4: play more, the last save wins
    data.highscore = 250;
    data.unlocked.push_back(3);
    expect_round_trip(path, data);

    expect_loads(path, player_save{"Player1", 250, {1, 2, 3}});
}

TEST_F(IntegrationTests, MultipleFiles)
{
    const player_save first{"first", 10, {1}};
    const player_save second{"second", 20, {2, 2}};
    const player_save third{"third", 30, {}};

    const auto first_path = m_dir / "first.sav";
    const auto second_path = m_dir / "second.sav";
    const auto third_path = m_dir / "third.sav";

    // Test 1: independent files hold different data
    expect_round_trip(first_path, first);
    expect_round_trip(second_path, second);
    expect_round_trip(third_path, third);

    // Test 2: saving one file doesn't change the others
    const player_save changed{"changed", 99, {9}};
    expect_round_trip(second_path, changed);

    expect_loads(first_path, first);
    expect_loads(second_path, changed);
    expect_loads(third_path, third);

    // Test 3: Encrypted, encrypted and plain files side by side in one directory
    {
        const player_save secret{"secret", 40, {4}};
        const auto secret_path = m_dir / "secret.sav";

        expect_round_trip(secret_path, secret, true);

        expect_loads(first_path, first);
        expect_loads(secret_path, secret);
    }
}

TEST_F(IntegrationTests, EncryptionToggleLifecycle)
{
    const auto path = m_dir / "toggle.sav";

    // Test 1: switch between plain and encrypted over several save and load cycles
    for (int i = 0; i < 6; ++i)
    {
        const bool encrypt = i % 2 == 1;
        const player_save data{"cycle", i, {i}};

        expect_round_trip(path, data, encrypt);
        EXPECT_EQ(read_text(path).starts_with("DATACOE_ENCRYPTED"), encrypt);
    }
}

//==============================================================================
//                              Corruption Recovery
//==============================================================================

TEST_F(IntegrationTests, CorruptionRecovery)
{
    // Test 1: garbage over a plain save, load fails and saving fresh data recovers
    {
        const auto path = m_dir / "plain.sav";
        expect_round_trip(path, player_save{"valid", 100, {}});

        write_text(path, "This is corrupted data that can't be parsed");
        EXPECT_FALSE(load<player_save>(path).has_value());

        expect_round_trip(path, player_save{"recovered", 300, {}});
    }

    // Test 2: Encrypted, truncated to the prefix, load fails and saving fresh data recovers
    {
        const auto path = m_dir / "encrypted.sav";
        expect_round_trip(path, player_save{"valid", 100, {}}, true);

        write_text(path, "DATACOE_ENCRYPTED");
        EXPECT_FALSE(load<player_save>(path).has_value());

        expect_round_trip(path, player_save{"recovered", 300, {}}, true);
    }
}
