#include <datacoe/utils/error.hpp>
#include <datacoe/utils/io.hpp>
#include <datacoe_config.hpp>
#include <support/test_support.hpp>
#include <gtest/gtest.h>
#include <cstddef>
#include <filesystem>
#include <string>
#include <system_error>

using namespace datacoe;
using namespace datacoe_test;

//==============================================================================
//                   IoTests - internal I/O layer tests
//==============================================================================

class IoTests : public ::testing::Test
{
protected:
    temp_dir m_dir;
};

//==============================================================================
//                                is_encrypted
//==============================================================================

TEST_F(IoTests, IsEncrypted)
{
    // Test 1: plain JSON
    EXPECT_FALSE(internal::is_encrypted(GOLDEN_PLAIN_JSON));

    // Test 2: the prefix alone is enough, no CryptoPP involved
    EXPECT_TRUE(internal::is_encrypted(ENCRYPTED_PREFIX));

    // Test 3: same length as the prefix but different text
    EXPECT_FALSE(internal::is_encrypted("DATACOE_ENCRYPTEX"));

    // Test 4: empty contents
    EXPECT_FALSE(internal::is_encrypted(""));

    // Test 5: shorter than the prefix
    EXPECT_FALSE(internal::is_encrypted("DATACOE"));
}

TEST_F(IoTests, IsEncryptedOnEncryptOutput)
{
    if constexpr (!DATACOE_ENABLE_ENCRYPTION)
        GTEST_SKIP() << "needs DATACOE_ENABLE_ENCRYPTION";

    // Test 1: output of encrypt()
    {
        const auto encrypted = internal::encrypt("{}");
        ASSERT_TRUE(encrypted.has_value()) << encrypted.error().message;
        EXPECT_TRUE(internal::is_encrypted(*encrypted));
    }
}

//==============================================================================
//                              encrypt and decrypt
//==============================================================================

TEST_F(IoTests, EncryptDecrypt)
{
    if constexpr (!DATACOE_ENABLE_ENCRYPTION)
        GTEST_SKIP() << "needs DATACOE_ENABLE_ENCRYPTION";

    // Test 1: round trip for different kinds of text
    {
        std::string long_text;
        for (std::size_t i = 0; i < 10 * 1024; ++i)
            long_text.push_back(static_cast<char>('a' + i % 26));

        const std::string texts[] = {
            "short text",
            "",
            std::string(16, 'a'),
            std::string(17, 'b'),
            long_text,
            "h\xc3\xa9llo w\xc3\xb6rld \xe2\x82\xac",
        };

        for (const auto &text : texts)
        {
            const auto encrypted = internal::encrypt(text);
            ASSERT_TRUE(encrypted.has_value()) << encrypted.error().message;
            EXPECT_TRUE(encrypted->starts_with(ENCRYPTED_PREFIX));

            const auto decrypted = internal::decrypt(*encrypted);
            ASSERT_TRUE(decrypted.has_value()) << decrypted.error().message;
            EXPECT_EQ(*decrypted, text);
        }
    }

    // Test 2: random IV, same plaintext gives different output
    {
        const auto first = internal::encrypt("same text");
        const auto second = internal::encrypt("same text");
        ASSERT_TRUE(first.has_value()) << first.error().message;
        ASSERT_TRUE(second.has_value()) << second.error().message;

        EXPECT_NE(*first, *second);
    }

    // Test 3: no prefix
    expect_error(internal::decrypt("not encrypted at all"), error_code::decryption_failure);

    // Test 4: prefix plus garbage
    expect_error(internal::decrypt("DATACOE_ENCRYPTED!!! garbage !!!"), error_code::decryption_failure);

    // Test 5: valid base64 but shorter than one block ("abcdefgh")
    expect_error(internal::decrypt("DATACOE_ENCRYPTEDYWJjZGVmZ2g="), error_code::decryption_failure);

    // Test 6: valid base64 of exactly one block, no ciphertext left ("0123456789abcdef")
    expect_error(internal::decrypt("DATACOE_ENCRYPTEDMDEyMzQ1Njc4OWFiY2RlZg=="), error_code::decryption_failure);

    // Test 7: valid base64 of 40 bytes, the ciphertext part is not a multiple of the block size
    expect_error(internal::decrypt("DATACOE_ENCRYPTEDeHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eA=="),
                 error_code::decryption_failure);
}

TEST_F(IoTests, GoldenOldSave)
{
    if constexpr (!DATACOE_ENABLE_ENCRYPTION)
        GTEST_SKIP() << "needs DATACOE_ENABLE_ENCRYPTION";

    // Test 1: a save from the first datacoe format still decrypts
    const auto decrypted = internal::decrypt(GOLDEN_ENCRYPTED_SAVE);
    ASSERT_TRUE(decrypted.has_value()) << decrypted.error().message;
    EXPECT_EQ(*decrypted, GOLDEN_PLAIN_JSON);
}

//==============================================================================
//                                  read_file
//==============================================================================

TEST_F(IoTests, ReadFile)
{
    // Test 1: binary round trip, newlines, a NUL and high bytes stay as they are
    {
        std::string bytes = "line1\nline2\r\nnul";
        bytes.push_back('\0');
        bytes += "high\xff\xfe\x80";

        const auto path = m_dir / "binary.dat";
        write_text(path, bytes);

        const auto result = internal::read_file(path);
        ASSERT_TRUE(result.has_value()) << result.error().message;
        EXPECT_EQ(*result, bytes);
    }

    // Test 2: empty file
    {
        const auto path = m_dir / "empty.dat";
        write_text(path, "");

        const auto result = internal::read_file(path);
        ASSERT_TRUE(result.has_value()) << result.error().message;
        EXPECT_TRUE(result->empty());
    }

    // Test 3: nonexistent file
    expect_error(internal::read_file(m_dir / "missing.dat"), error_code::file_not_found);

    // Test 4: directory
    expect_error(internal::read_file(m_dir.path()), error_code::file_read_failure);

    // Test 5: empty path
    expect_error(internal::read_file(std::filesystem::path()), error_code::invalid_argument);
}

//==============================================================================
//                                  write_file
//==============================================================================

TEST_F(IoTests, WriteFile)
{
    // Test 1: creates the file with the exact contents and leaves no temp file
    {
        const auto path = m_dir / "save.json";
        const std::string contents = "first\r\n\xff";

        const auto result = internal::write_file(path, contents);
        ASSERT_TRUE(result.has_value()) << result.error().message;

        EXPECT_EQ(read_text(path), contents);
        EXPECT_FALSE(std::filesystem::exists(tmp_path(path)));
    }

    // Test 2: overwrites an existing file
    {
        const auto path = m_dir / "overwrite.json";
        write_text(path, "a much longer old content");

        const auto result = internal::write_file(path, "new");
        ASSERT_TRUE(result.has_value()) << result.error().message;

        EXPECT_EQ(read_text(path), "new");
        EXPECT_FALSE(std::filesystem::exists(tmp_path(path)));
    }

    // Test 3: empty contents
    {
        const auto path = m_dir / "empty.json";

        const auto result = internal::write_file(path, "");
        ASSERT_TRUE(result.has_value()) << result.error().message;

        EXPECT_TRUE(std::filesystem::exists(path));
        EXPECT_TRUE(read_text(path).empty());
    }

    // Test 4: empty path
    expect_error(internal::write_file(std::filesystem::path(), "data"), error_code::invalid_argument);

    // Test 5: path without a file name
    expect_error(internal::write_file(m_dir.path() / ".", "data"), error_code::invalid_argument);
    expect_error(internal::write_file(m_dir.path() / "..", "data"), error_code::invalid_argument);

    // Test 6: nonexistent directory, nothing gets created
    {
        const auto path = m_dir / "missing_dir" / "save.json";

        expect_error(internal::write_file(path, "data"), error_code::file_write_failure);
        EXPECT_FALSE(std::filesystem::exists(m_dir / "missing_dir"));
    }

    // Test 7: stale temp file from an earlier crash is overwritten and gone afterwards
    {
        const auto path = m_dir / "stale.json";
        write_text(tmp_path(path), "garbage from a crash");

        const auto result = internal::write_file(path, "fresh");
        ASSERT_TRUE(result.has_value()) << result.error().message;

        EXPECT_EQ(read_text(path), "fresh");
        EXPECT_FALSE(std::filesystem::exists(tmp_path(path)));
    }
}

TEST_F(IoTests, WriteFileFailureKeepsExistingSave)
{
    std::error_code ec;

    // Test 1: temp path is a directory, the write fails and the original stays
    {
        const auto path = m_dir / "blocked_tmp.json";
        const auto result = internal::write_file(path, "original");
        ASSERT_TRUE(result.has_value()) << result.error().message;

        std::filesystem::create_directory(tmp_path(path), ec);
        ASSERT_FALSE(ec) << ec.message();

        expect_error(internal::write_file(path, "new"), error_code::file_write_failure);

        const auto contents = internal::read_file(path);
        ASSERT_TRUE(contents.has_value()) << contents.error().message;
        EXPECT_EQ(*contents, "original");
        EXPECT_TRUE(std::filesystem::is_directory(tmp_path(path)));
    }

    // Test 2: the target is a directory, the rename fails and the temp file is removed
    {
        const auto path = m_dir / "blocked_target.json";
        std::filesystem::create_directory(path, ec);
        ASSERT_FALSE(ec) << ec.message();

        expect_error(internal::write_file(path, "new"), error_code::file_write_failure);

        EXPECT_FALSE(std::filesystem::exists(tmp_path(path)));
        EXPECT_TRUE(std::filesystem::is_directory(path));
    }
}
