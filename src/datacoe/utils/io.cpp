#include <datacoe/utils/io.hpp>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cstddef>
#include <cstring>
#include <exception>
#include <format>
#include <fstream>
#include <iterator>
#include <string_view>
#include <system_error>

namespace datacoe::internal
{
    namespace
    {
        constexpr std::string_view ENCRYPTION_PREFIX = "DATACOE_ENCRYPTED";

        // Fixed Encryption Key (Warning: This is Insecure, I'm using it for learning purposes only!)
        constexpr CryptoPP::byte FIXED_KEY[] = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

        // path.string() can throw on MSVC, u8string() can't
        std::string path_text(const std::filesystem::path &path)
        {
            const auto text = path.u8string();
            return std::string(text.begin(), text.end());
        }

        std::expected<void, error> check_readable_file(const std::filesystem::path &path)
        {
            if (path.empty())
                return std::unexpected(error{error_code::invalid_argument, "path is empty"});

            std::error_code ec;
            if (!std::filesystem::exists(path, ec))
                return std::unexpected(
                    error{error_code::file_not_found, std::format("file not found: {}", path_text(path))});

            if (!std::filesystem::is_regular_file(path, ec))
                return std::unexpected(
                    error{error_code::file_read_failure, std::format("not a regular file: {}", path_text(path))});

            return {};
        }
    } // namespace

    std::expected<bool, error> is_file_encrypted(const std::filesystem::path &path)
    {
        try
        {
            const auto readable = check_readable_file(path);
            if (!readable)
                return std::unexpected(readable.error());

            std::ifstream file(path, std::ios::binary);
            if (!file.is_open())
                return std::unexpected(
                    error{error_code::file_read_failure, std::format("could not open file: {}", path_text(path))});

            std::string header(ENCRYPTION_PREFIX.size(), '\0');
            file.read(header.data(), static_cast<std::streamsize>(header.size()));
            if (file.bad())
                return std::unexpected(
                    error{error_code::file_read_failure, std::format("could not read file: {}", path_text(path))});

            return static_cast<std::size_t>(file.gcount()) == header.size() && header == ENCRYPTION_PREFIX;
        }
        catch (const std::exception &e)
        {
            return std::unexpected(error{error_code::file_read_failure, e.what()});
        }
    }

    std::expected<std::string, error> encrypt(const std::string &plaintext)
    {
        try
        {
            CryptoPP::AutoSeededRandomPool rng;
            CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
            rng.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);

            // The filters take ownership of the raw pointers, that's the CryptoPP API
            std::string ciphertext;
            CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryption(FIXED_KEY, CryptoPP::AES::DEFAULT_KEYLENGTH, iv);
            CryptoPP::StringSource ss1(plaintext, true,
                                       new CryptoPP::StreamTransformationFilter(encryption,
                                                                                new CryptoPP::StringSink(ciphertext)));

            std::string combined(reinterpret_cast<const char *>(iv), CryptoPP::AES::BLOCKSIZE);
            combined += ciphertext;

            // The default Base64Encoder adds a line break every 72 chars. Keep it, old saves have them
            std::string encoded;
            CryptoPP::StringSource ss2(combined, true,
                                       new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded)));

            return std::string(ENCRYPTION_PREFIX) + encoded;
        }
        catch (const std::exception &e)
        {
            return std::unexpected(error{error_code::encryption_failure, e.what()});
        }
    }

    std::expected<std::string, error> decrypt(const std::string &file_contents)
    {
        try
        {
            if (!file_contents.starts_with(ENCRYPTION_PREFIX))
                return std::unexpected(error{error_code::decryption_failure, "missing encryption prefix"});

            std::string decoded;
            CryptoPP::StringSource ss1(file_contents.substr(ENCRYPTION_PREFIX.size()), true,
                                       new CryptoPP::Base64Decoder(new CryptoPP::StringSink(decoded)));

            if (decoded.size() <= CryptoPP::AES::BLOCKSIZE)
                return std::unexpected(error{error_code::decryption_failure, "encrypted data is too short"});

            CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
            std::memcpy(iv, decoded.data(), CryptoPP::AES::BLOCKSIZE);
            const std::string ciphertext = decoded.substr(CryptoPP::AES::BLOCKSIZE);

            std::string plaintext;
            CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryption(FIXED_KEY, CryptoPP::AES::DEFAULT_KEYLENGTH, iv);
            CryptoPP::StringSource ss2(ciphertext, true,
                                       new CryptoPP::StreamTransformationFilter(decryption,
                                                                                new CryptoPP::StringSink(plaintext)));

            return plaintext;
        }
        catch (const std::exception &e)
        {
            return std::unexpected(error{error_code::decryption_failure, e.what()});
        }
    }

    std::expected<std::string, error> read_file(const std::filesystem::path &path)
    {
        try
        {
            const auto readable = check_readable_file(path);
            if (!readable)
                return std::unexpected(readable.error());

            std::ifstream file(path, std::ios::binary);
            if (!file.is_open())
                return std::unexpected(
                    error{error_code::file_read_failure, std::format("could not open file: {}", path_text(path))});

            std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            if (file.bad())
                return std::unexpected(
                    error{error_code::file_read_failure, std::format("could not read file: {}", path_text(path))});

            return contents;
        }
        catch (const std::exception &e)
        {
            return std::unexpected(error{error_code::file_read_failure, e.what()});
        }
    }

    // The old save is only replaced by the final rename, so a crash or a write error never destroys it.
    // A power cut can still leave a partial file after the rename, the standard library has no fsync.
    // Concurrent saves to the same path share <path>.tmp and are not supported.
    std::expected<void, error> write_file(const std::filesystem::path &path, const std::string &contents)
    {
        try
        {
            const auto filename = path.filename();
            if (filename.empty() || filename == "." || filename == "..")
                return std::unexpected(
                    error{error_code::invalid_argument, std::format("path has no file name: {}", path_text(path))});

            auto tmp = path;
            tmp += ".tmp";

            std::ofstream file(tmp, std::ios::binary | std::ios::trunc);
            if (!file.is_open())
                return std::unexpected(error{error_code::file_write_failure,
                                             std::format("could not open file: {}", path_text(tmp))});

            file.write(contents.data(), static_cast<std::streamsize>(contents.size()));
            file.flush();
            const bool written = file.good();
            file.close();

            std::error_code ec;
            if (!written || file.fail())
            {
                std::filesystem::remove(tmp, ec);
                return std::unexpected(error{error_code::file_write_failure,
                                             std::format("could not write file: {}", path_text(tmp))});
            }

            std::filesystem::rename(tmp, path, ec);
            if (ec)
            {
                std::error_code ignored;
                std::filesystem::remove(tmp, ignored);
                return std::unexpected(error{error_code::file_write_failure,
                                             std::format("could not replace {}: {}", path_text(path), ec.message())});
            }

            return {};
        }
        catch (const std::exception &e)
        {
            return std::unexpected(error{error_code::file_write_failure, e.what()});
        }
    }
} // namespace datacoe::internal
