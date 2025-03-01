#include "gtest/gtest.h"
#include "data_reader_writer.hpp"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <iostream>

namespace datacoe
{

    class DataReaderWriterTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            m_testFilename = "test_data_rw.data";
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
            // Give a small delay to ensure file handles are released
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Try to clean up the test file
            for (int i = 0; i < 5; i++)
            {
                try
                {
                    if (std::filesystem::exists(m_testFilename))
                    {
                        std::filesystem::remove(m_testFilename);
                    }
                    break;
                }
                catch (const std::filesystem::filesystem_error &e)
                {
                    std::cerr << "TearDown() Error: " << e.what() << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }

        std::string m_testFilename;
    };

    TEST_F(DataReaderWriterTest, WriteAndReadData)
    {
        try
        {
            // Create test data
            GameData gd("TestData", 200);

            // Write data to file
            bool writeResult = DataReaderWriter::writeData(gd, m_testFilename);
            ASSERT_TRUE(writeResult) << "Failed to write data to file";

            // Verify file exists
            ASSERT_TRUE(std::filesystem::exists(m_testFilename)) << "File was not created";

            // Read data from file
            std::optional<GameData> loadedData = DataReaderWriter::readData(m_testFilename);
            ASSERT_TRUE(loadedData.has_value()) << "Failed to read data from file";

            // Verify data
            ASSERT_EQ(loadedData.value().getNickName(), "TestData");
            ASSERT_EQ(loadedData.value().getHighscore(), 200);
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

    TEST_F(DataReaderWriterTest, WriteAndReadSpecialCharacters)
    {
        try
        {
            // Test with special characters in the nickname
            GameData gd("Test@Data#$%^&*", 300);

            ASSERT_TRUE(DataReaderWriter::writeData(gd, m_testFilename));

            std::optional<GameData> loadedData = DataReaderWriter::readData(m_testFilename);
            ASSERT_TRUE(loadedData.has_value());

            ASSERT_EQ(loadedData.value().getNickName(), "Test@Data#$%^&*");
            ASSERT_EQ(loadedData.value().getHighscore(), 300);
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception with special characters: " << e.what();
        }
    }

    TEST_F(DataReaderWriterTest, ReadDataFail)
    {
        std::optional<GameData> loadedData = DataReaderWriter::readData("non_existent_file.json");
        ASSERT_FALSE(loadedData.has_value()) << "Expected failure on non-existent file";
    }

    TEST_F(DataReaderWriterTest, WriteFailInvalidPath)
    {
        GameData gd("TestData", 400);
        bool result = DataReaderWriter::writeData(gd, "/invalid/path/file.json");
        ASSERT_FALSE(result) << "Expected failure on invalid file path";
    }

    TEST_F(DataReaderWriterTest, ReadCorruptedFile)
    {
        // Create a corrupted file
        {
            std::ofstream file(m_testFilename);
            file << "This is not valid encrypted data";
            file.close();
        }

        // Try to read it
        std::optional<GameData> loadedData = DataReaderWriter::readData(m_testFilename);
        ASSERT_FALSE(loadedData.has_value()) << "Expected failure on corrupted file";
    }

} // namespace datacoe