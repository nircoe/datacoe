#pragma once

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#ifdef _WIN32
    #include <process.h>
#else
    #include <unistd.h>
#endif

namespace datacoe_test
{
    struct player_save
    {
        std::string nickname;
        int highscore = 0;
        std::vector<int> unlocked;

        bool operator==(const player_save &) const = default;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(player_save, nickname, highscore, unlocked)

    struct throwing_save
    {
        int value = 0;
    };

    inline void to_json(nlohmann::json &, const throwing_save &)
    {
        throw std::runtime_error("throwing_save to_json");
    }

    inline void from_json(const nlohmann::json &, throwing_save &)
    {
        throw std::runtime_error("throwing_save from_json");
    }

    // No to_json/from_json on purpose, for the negative concept checks
    struct plain_struct
    {
        int a = 0;
    };

    // Has to_json/from_json, so the only thing missing for saveable is the default constructor
    struct no_default_save
    {
        explicit no_default_save(int);
    };

    inline void to_json(nlohmann::json &, const no_default_save &)
    {
    }

    inline void from_json(const nlohmann::json &, no_default_save &)
    {
    }

    inline int process_id()
    {
#ifdef _WIN32
        return _getpid();
#else
        return getpid();
#endif
    }

    // Unique directory per process, suite and test, removed again when it goes out of scope
    class temp_dir
    {
        std::filesystem::path m_path;

    public:
        temp_dir()
        {
            const auto *info = ::testing::UnitTest::GetInstance()->current_test_info();
            m_path = std::filesystem::temp_directory_path() /
                     std::format("datacoe_tests_{}_{}_{}", process_id(), info->test_suite_name(), info->name());

            std::error_code ec;
            std::filesystem::remove_all(m_path, ec);
            std::filesystem::create_directories(m_path, ec);
        }

        temp_dir(const temp_dir &) = delete;
        temp_dir &operator=(const temp_dir &) = delete;

        ~temp_dir()
        {
            std::error_code ec;
            std::filesystem::remove_all(m_path, ec);
        }

        const std::filesystem::path &path() const noexcept { return m_path; }

        std::filesystem::path operator/(const std::string &name) const { return m_path / name; }
    };

    inline std::string read_text(const std::filesystem::path &path)
    {
        std::ifstream file(path, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    inline void write_text(const std::filesystem::path &path, const std::string &text)
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file.write(text.data(), static_cast<std::streamsize>(text.size()));
    }

    // Real encrypted save from the first datacoe format, it has to keep decrypting
    inline const std::string GOLDEN_ENCRYPTED_SAVE =
        "DATACOE_ENCRYPTED"
        "V8sE3TRe59sbOtQANeoLtYM6QEvk9PS0XKGBlXJ7zNTd3sadxou/aF6hgyr2auTOMkp8a5mT\n"
        "ctT8c1rX0d4fDw==\n";

    inline const std::string GOLDEN_PLAIN_JSON = R"({"highscore":7,"nickname":"golden"})";
} // namespace datacoe_test
