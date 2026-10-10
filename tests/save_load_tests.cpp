#include <datacoe/save_load.hpp>
#include <support/test_support.hpp>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <climits>
#include <expected>
#include <filesystem>
#include <string>
#include <system_error>

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

    std::filesystem::path tmp_path(const std::filesystem::path &path)
    {
        auto tmp = path;
        tmp += ".tmp";
        return tmp;
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
//                    SaveLoadTests - save and load API tests
//==============================================================================

class SaveLoadTests : public ::testing::Test
{
protected:
    temp_dir m_dir;
};

//==============================================================================
//                              Plain Round Trip
//==============================================================================

TEST_F(SaveLoadTests, PlainRoundTrip)
{
    // Test 1: save then load, then overwrite with new data
    {
        const auto path = m_dir / "save.json";

        expect_round_trip(path, player_save{"nir", 42, {}});
        expect_round_trip(path, player_save{"second", 7, {}});
    }

    // Test 2: special characters and unicode in the nickname
    expect_round_trip(m_dir / "special.json",
                      player_save{"Zo\xc3\xab \"quoted\" \\ back\nline\ttab \xe2\x82\xac", 1, {}});
    expect_round_trip(m_dir / "emoji.json", player_save{"\xf0\x9f\x8e\xae player", 1, {}});

    // Test 3: empty nickname
    expect_round_trip(m_dir / "empty.json", player_save{"", 5, {}});

    // Test 4: biggest highscore
    expect_round_trip(m_dir / "max.json", player_save{"max", INT_MAX, {}});

    // Test 5: 100 KB nickname
    expect_round_trip(m_dir / "big.json", player_save{std::string(100 * 1024, 'x'), 1, {}});

    // Test 6: populated vector
    expect_round_trip(m_dir / "unlocked.json", player_save{"collector", 3, {1, 2, 3, 5, 8, 13}});

    // Test 7: the file on disk is readable JSON and no temp file is left
    {
        const auto path = m_dir / "readable.json";
        const auto saved = save(path, player_save{"readable", 9, {}});
        ASSERT_TRUE(saved.has_value()) << saved.error().message;

        const auto text = read_text(path);
        EXPECT_NE(text.find("\"nickname\""), std::string::npos);
        EXPECT_TRUE(nlohmann::json::accept(text));
        EXPECT_FALSE(std::filesystem::exists(tmp_path(path)));
    }
}

//==============================================================================
//                                  Encryption
//==============================================================================

TEST_F(SaveLoadTests, EncryptedRoundTrip)
{
    // Test 1: file is encrypted on disk and load needs no flag
    {
        const auto path = m_dir / "encrypted.sav";
        const player_save data{"secret_nickname", 99, {4, 5}};

        expect_round_trip(path, data, true);

        const auto text = read_text(path);
        EXPECT_TRUE(text.starts_with("DATACOE_ENCRYPTED"));
        EXPECT_EQ(text.find("secret_nickname"), std::string::npos);
    }
}

TEST_F(SaveLoadTests, EncryptionSwitching)
{
    const auto path = m_dir / "switching.sav";

    // Test 1: plain, then encrypted, then plain again on the same path
    {
        expect_round_trip(path, player_save{"plain", 1, {}});
        EXPECT_FALSE(read_text(path).starts_with("DATACOE_ENCRYPTED"));

        expect_round_trip(path, player_save{"encrypted", 2, {}}, true);
        EXPECT_TRUE(read_text(path).starts_with("DATACOE_ENCRYPTED"));

        expect_round_trip(path, player_save{"plain again", 3, {}});
        EXPECT_FALSE(read_text(path).starts_with("DATACOE_ENCRYPTED"));
    }
}

TEST_F(SaveLoadTests, GoldenOldEncryptedSave)
{
    // Test 1: a save from the first datacoe format still loads
    {
        const auto path = m_dir / "golden.sav";
        write_text(path, GOLDEN_ENCRYPTED_SAVE);

        const auto loaded = load<player_save>(path);
        ASSERT_TRUE(loaded.has_value()) << loaded.error().message;
        EXPECT_EQ(loaded->nickname, "golden");
        EXPECT_EQ(loaded->highscore, 7);
        EXPECT_TRUE(loaded->unlocked.empty());
    }
}

//==============================================================================
//                              Missing and Extra Keys
//==============================================================================

TEST_F(SaveLoadTests, MissingAndExtraKeys)
{
    // Test 1: missing keys fall back to the defaults
    {
        const auto path = m_dir / "missing.json";
        write_text(path, R"({"nickname":"x"})");

        expect_loads(path, player_save{"x", 0, {}});
    }

    // Test 2: unknown keys are ignored
    {
        const auto path = m_dir / "extra.json";
        write_text(path, R"({"nickname":"x","highscore":3,"unlocked":[1],"from_the_future":true})");

        expect_loads(path, player_save{"x", 3, {1}});
    }
}

//==============================================================================
//                                 Error Mapping
//==============================================================================

TEST_F(SaveLoadTests, ErrorMapping)
{
    // Test 1: nonexistent file, the message carries the load prefix (internal I/O error)
    {
        const auto result = load<player_save>(m_dir / "missing.json");

        expect_error(result, error_code::file_not_found);
        EXPECT_TRUE(result.error().message.starts_with("datacoe::load()"));
    }

    // Test 2: not JSON, the message carries the load prefix (parse error)
    {
        const auto path = m_dir / "not_json.json";
        write_text(path, "not json");

        const auto result = load<player_save>(path);

        expect_error(result, error_code::parse_failure);
        EXPECT_TRUE(result.error().message.starts_with("datacoe::load()"));
    }

    // Test 3: JSON of the wrong shape
    {
        const auto array_path = m_dir / "array.json";
        write_text(array_path, "[1,2]");
        expect_error(load<player_save>(array_path), error_code::deserialization_failure);

        const auto type_path = m_dir / "wrong_type.json";
        write_text(type_path, R"({"highscore":"abc"})");
        expect_error(load<player_save>(type_path), error_code::deserialization_failure);
    }

    // Test 4: Encrypted, prefix plus garbage
    {
        const auto path = m_dir / "garbage.sav";
        write_text(path, "DATACOE_ENCRYPTED!!! garbage !!!");

        expect_error(load<player_save>(path), error_code::decryption_failure);
    }

    // Test 5: to_json throws, the message carries the save prefix (serialization error)
    {
        const auto path = m_dir / "throwing.json";
        const auto result = save(path, throwing_save{});

        expect_error(result, error_code::serialization_failure);
        EXPECT_TRUE(result.error().message.starts_with("datacoe::save()"));
        EXPECT_FALSE(std::filesystem::exists(path));
    }

    // Test 6: from_json throws on a valid file
    {
        const auto path = m_dir / "throwing_load.json";
        write_text(path, "{}");

        expect_error(load<throwing_save>(path), error_code::deserialization_failure);
    }

    // Test 7: invalid UTF-8, dump() throws in strict mode
    expect_error(save(m_dir / "utf8.json", player_save{"\xff", 1, {}}), error_code::serialization_failure);

    // Test 8: empty path
    expect_error(save(std::filesystem::path(), player_save{}), error_code::invalid_argument);
    expect_error(load<player_save>(std::filesystem::path()), error_code::invalid_argument);

    // Test 9: nonexistent directory, the message carries the save prefix (internal I/O error)
    {
        const auto result = save(m_dir / "missing_dir" / "save.json", player_save{});

        expect_error(result, error_code::file_write_failure);
        EXPECT_TRUE(result.error().message.starts_with("datacoe::save()"));
    }

    // Test 10: loading a directory
    expect_error(load<player_save>(m_dir.path()), error_code::file_read_failure);
}

//==============================================================================
//                          Failed Writes Keep the Old Save
//==============================================================================

TEST_F(SaveLoadTests, FailedWriteKeepsExistingSave)
{
    std::error_code ec;
    const player_save original{"original", 1, {1, 2}};

    // Test 1: the temp path is a directory, so the write fails and the original stays
    {
        const auto path = m_dir / "blocked_tmp.json";
        expect_round_trip(path, original);

        std::filesystem::create_directory(tmp_path(path), ec);
        ASSERT_FALSE(ec) << ec.message();

        expect_error(save(path, player_save{"new", 2, {}}), error_code::file_write_failure);
        expect_loads(path, original);
    }

    // Test 2: serialization fails before the disk is touched
    {
        const auto path = m_dir / "bad_utf8.json";
        expect_round_trip(path, original);

        expect_error(save(path, player_save{"\xff", 2, {}}), error_code::serialization_failure);
        expect_loads(path, original);
        EXPECT_FALSE(std::filesystem::exists(tmp_path(path)));
    }
}

TEST_F(SaveLoadTests, FailedWriteKeepsExistingEncryptedSave)
{
    std::error_code ec;
    const player_save original{"original", 1, {1, 2}};

    // Test 1: the temp path is a directory, so the write fails and the original stays
    {
        const auto path = m_dir / "blocked_tmp.sav";
        expect_round_trip(path, original, true);

        std::filesystem::create_directory(tmp_path(path), ec);
        ASSERT_FALSE(ec) << ec.message();

        expect_error(save(path, player_save{"new", 2, {}}, true), error_code::file_write_failure);
        expect_loads(path, original);
    }

    // Test 2: serialization fails before the disk is touched
    {
        const auto path = m_dir / "bad_utf8.sav";
        expect_round_trip(path, original, true);

        expect_error(save(path, player_save{"\xff", 2, {}}, true), error_code::serialization_failure);
        expect_loads(path, original);
        EXPECT_FALSE(std::filesystem::exists(tmp_path(path)));
    }
}
