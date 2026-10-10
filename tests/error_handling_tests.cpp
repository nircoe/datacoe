#include <datacoe/save_load.hpp>
#include <support/test_support.hpp>
#include <gtest/gtest.h>
#include <expected>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>

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

    void expect_loads(const std::filesystem::path &path, const player_save &expected)
    {
        const auto loaded = load<player_save>(path);
        ASSERT_TRUE(loaded.has_value()) << loaded.error().message;
        EXPECT_EQ(*loaded, expected);
    }

    void expect_resave_recovers(const std::filesystem::path &path, bool encrypt = false)
    {
        const player_save fresh{"recovered", 5, {1}};

        const auto saved = save(path, fresh, encrypt);
        ASSERT_TRUE(saved.has_value()) << saved.error().message;
        expect_loads(path, fresh);
    }

    // Puts write permission back, a failing ASSERT must not leave a read-only file for remove_all
    class write_permission_guard
    {
        std::filesystem::path m_path;

    public:
        explicit write_permission_guard(std::filesystem::path path) : m_path(std::move(path)) {}

        write_permission_guard(const write_permission_guard &) = delete;
        write_permission_guard &operator=(const write_permission_guard &) = delete;

        ~write_permission_guard()
        {
            std::error_code ec;
            std::filesystem::permissions(m_path, std::filesystem::perms::owner_write,
                                         std::filesystem::perm_options::add, ec);
        }
    };
} // namespace

//==============================================================================
//                ErrorHandlingTests - failure and recovery tests
//==============================================================================

class ErrorHandlingTests : public ::testing::Test
{
protected:
    temp_dir m_dir;
};

//==============================================================================
//                                Corrupt Files
//==============================================================================

TEST_F(ErrorHandlingTests, CorruptFiles)
{
    // Test 1: truncated JSON
    {
        const auto path = m_dir / "truncated.json";
        write_text(path, "{");

        expect_error(load<player_save>(path), error_code::parse_failure);
        expect_resave_recovers(path);
    }

    // Test 2: Encrypted, cut in the middle of the base64 text
    {
        const auto path = m_dir / "truncated.sav";
        const auto saved = save(path, player_save{std::string(100, 'n'), 1, {}}, true);
        ASSERT_TRUE(saved.has_value()) << saved.error().message;

        // The prefix plus the first base64 line (72 chars) decodes to a size that isn't a multiple of the block size
        const auto text = read_text(path);
        write_text(path, text.substr(0, 17 + 72));

        expect_error(load<player_save>(path), error_code::decryption_failure);
        expect_resave_recovers(path, true);
    }

    // Test 3: valid JSON with an unrelated structure loads with the defaults
    {
        const auto path = m_dir / "unrelated.json";
        write_text(path, R"({"wrongKey":"wrongValue"})");

        expect_loads(path, player_save{});
        expect_resave_recovers(path);
    }
}

//==============================================================================
//                                Bad Locations
//==============================================================================

TEST_F(ErrorHandlingTests, ParentIsAFile)
{
    const auto parent = m_dir / "not_a_directory";
    write_text(parent, "just a file");
    const auto path = parent / "save.json";

    // Test 1: save can't create the temp file
    expect_error(save(path, player_save{}), error_code::file_write_failure);

    // Test 2: the exact code differs between Linux and Windows
    EXPECT_FALSE(load<player_save>(path).has_value());
}

TEST_F(ErrorHandlingTests, ReadOnlyTarget)
{
    const auto path = m_dir / "readonly.sav";
    const player_save original{"original", 1, {}};
    const player_save updated{"updated", 2, {}};

    const auto first = save(path, original);
    ASSERT_TRUE(first.has_value()) << first.error().message;

    // Renaming over a read-only file succeeds on POSIX and fails on Windows, so only the invariant is checked
    write_permission_guard guard(path);

    std::error_code ec;
    std::filesystem::permissions(path,
                                 std::filesystem::perms::owner_write | std::filesystem::perms::group_write |
                                     std::filesystem::perms::others_write,
                                 std::filesystem::perm_options::remove, ec);
    if (ec)
        GTEST_SKIP() << "could not make the file read-only: " << ec.message();

    const auto result = save(path, updated);

    expect_loads(path, result.has_value() ? updated : original);
    EXPECT_FALSE(std::filesystem::exists(tmp_path(path)));
}

//==============================================================================
//                                Interrupted Save
//==============================================================================

TEST_F(ErrorHandlingTests, InterruptedSave)
{
    const player_save original{"original", 1, {1}};
    const player_save updated{"updated", 2, {2}};

    // Test 1: leftover temp file with garbage, the original still loads and the next save cleans up
    {
        const auto path = m_dir / "leftover.sav";
        const auto saved = save(path, original);
        ASSERT_TRUE(saved.has_value()) << saved.error().message;
        write_text(tmp_path(path), "garbage from a crash");

        expect_loads(path, original);

        const auto resaved = save(path, updated);
        ASSERT_TRUE(resaved.has_value()) << resaved.error().message;
        expect_loads(path, updated);
        EXPECT_FALSE(std::filesystem::exists(tmp_path(path)));
    }

    // Test 2: half-written temp file
    {
        const auto path = m_dir / "half_written.sav";
        const auto saved = save(path, original);
        ASSERT_TRUE(saved.has_value()) << saved.error().message;

        const auto text = read_text(path);
        write_text(tmp_path(path), text.substr(0, text.size() / 2));

        expect_loads(path, original);

        const auto resaved = save(path, updated);
        ASSERT_TRUE(resaved.has_value()) << resaved.error().message;
        expect_loads(path, updated);
        EXPECT_FALSE(std::filesystem::exists(tmp_path(path)));
    }
}
