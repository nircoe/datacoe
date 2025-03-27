#include "gtest/gtest.h"
#include "datacoe/data_reader_writer.hpp"
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
        std::string m_testFilename;

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
            ASSERT_EQ(loadedData.value().getNickname(), "TestData");
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

            ASSERT_EQ(loadedData.value().getNickname(), "Test@Data#$%^&*");
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

    TEST_F(DataReaderWriterTest, WriteAndReadDataWithAutoDetection)
    {
        std::string unencryptedFilename = m_testFilename + ".unencrypted";

        try
        {
            // Create test data
            GameData originalData("AutoDetectTest", 300);

            // Write with encryption
            bool writeEncryptedResult = DataReaderWriter::writeData(originalData, m_testFilename, true);
            ASSERT_TRUE(writeEncryptedResult) << "Failed to write encrypted data";
            ASSERT_TRUE(std::filesystem::exists(m_testFilename)) << "File was not created";
            ASSERT_TRUE(DataReaderWriter::isFileEncrypted(m_testFilename)) << "File should be detected as encrypted";

            // Try to read with decryption=false
            // Should auto-adjust to decryption=true based on file detection
            std::optional<GameData> loadedEncrypted = DataReaderWriter::readData(m_testFilename, false);
            ASSERT_TRUE(loadedEncrypted.has_value()) << "Failed to read encrypted data with auto-detection";

            // Verify data was read correctly
            ASSERT_EQ(loadedEncrypted.value().getNickname(), "AutoDetectTest");
            ASSERT_EQ(loadedEncrypted.value().getHighscore(), 300);

            // Now write the same data unencrypted to a new file
            bool writeUnencryptedResult = DataReaderWriter::writeData(originalData, unencryptedFilename, false);
            ASSERT_TRUE(writeUnencryptedResult) << "Failed to write unencrypted data";
            ASSERT_FALSE(DataReaderWriter::isFileEncrypted(unencryptedFilename)) << "File should be detected as unencrypted";

            // Try to read with decryption=true
            // Should auto-adjust to decryption=false based on file detection
            std::optional<GameData> loadedUnencrypted = DataReaderWriter::readData(unencryptedFilename, true);
            ASSERT_TRUE(loadedUnencrypted.has_value()) << "Failed to read unencrypted data with auto-detection";

            // Verify data was read correctly
            ASSERT_EQ(loadedUnencrypted.value().getNickname(), "AutoDetectTest");
            ASSERT_EQ(loadedUnencrypted.value().getHighscore(), 300);
        }
        catch (const std::exception &e)
        {
            // Clean up in exception path
            if (std::filesystem::exists(unencryptedFilename))
            {
                try
                {
                    std::filesystem::remove(unencryptedFilename);
                }
                catch (...)
                { /* Ignore cleanup errors */
                }
            }
            FAIL() << "Unexpected exception: " << e.what();
        }

        // Clean up in normal path
        if (std::filesystem::exists(unencryptedFilename))
        {
            try
            {
                std::filesystem::remove(unencryptedFilename);
            }
            catch (...)
            { /* Ignore cleanup errors */
            }
        }
    }

    TEST_F(DataReaderWriterTest, EncryptionDetection)
    {
        std::string unencryptedFilename = m_testFilename + ".unencrypted";
        std::string invalidFilename = m_testFilename + ".invalid";

        try
        {
            // Create test data
            GameData testData("EncryptionDetectionTest", 400);

            // Write encrypted data
            ASSERT_TRUE(DataReaderWriter::writeData(testData, m_testFilename, true));
            ASSERT_TRUE(DataReaderWriter::isFileEncrypted(m_testFilename)) << "File should be detected as encrypted";

            // Write unencrypted data to a different file
            ASSERT_TRUE(DataReaderWriter::writeData(testData, unencryptedFilename, false));
            ASSERT_FALSE(DataReaderWriter::isFileEncrypted(unencryptedFilename)) << "File should be detected as unencrypted";

            // Create a file with invalid data
            {
                std::ofstream file(invalidFilename);
                file << "This is not a valid encrypted or JSON file";
                file.close();
            }
            ASSERT_FALSE(DataReaderWriter::isFileEncrypted(invalidFilename)) << "Invalid file should not be detected as encrypted";

            // Test with non-existent file
            ASSERT_FALSE(DataReaderWriter::isFileEncrypted("non_existent_file.json")) << "Non-existent file should not be detected as encrypted";
        }
        catch (const std::exception &e)
        {
            // Clean up in exception path
            try
            {
                if (std::filesystem::exists(unencryptedFilename))
                    std::filesystem::remove(unencryptedFilename);
                if (std::filesystem::exists(invalidFilename))
                    std::filesystem::remove(invalidFilename);
            }
            catch (...)
            { /* Ignore cleanup errors */
            }

            FAIL() << "Unexpected exception: " << e.what();
        }

        // Clean up in normal path
        try
        {
            if (std::filesystem::exists(unencryptedFilename))
                std::filesystem::remove(unencryptedFilename);
            if (std::filesystem::exists(invalidFilename))
                std::filesystem::remove(invalidFilename);
        }
        catch (...)
        { /* Ignore cleanup errors */
        }
    }

    TEST_F(DataReaderWriterTest, WriteEncryptedReadUnencrypted)
    {
        try
        {
            // Create and write encrypted data
            GameData originalData("EncryptedData", 500);
            ASSERT_TRUE(DataReaderWriter::writeData(originalData, m_testFilename, true));

            // Try to read without decryption
            // This should auto-adjust and work correctly
            std::optional<GameData> loadedData = DataReaderWriter::readData(m_testFilename, false);
            ASSERT_TRUE(loadedData.has_value()) << "Should be able to read encrypted data with auto-detection";

            // Verify data
            ASSERT_EQ(loadedData.value().getNickname(), "EncryptedData");
            ASSERT_EQ(loadedData.value().getHighscore(), 500);
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }

    TEST_F(DataReaderWriterTest, WriteUnencryptedReadEncrypted)
    {
        try
        {
            // Create and write unencrypted data
            GameData originalData("UnencryptedData", 600);
            ASSERT_TRUE(DataReaderWriter::writeData(originalData, m_testFilename, false));

            // Try to read with decryption
            // This should auto-adjust and work correctly
            std::optional<GameData> loadedData = DataReaderWriter::readData(m_testFilename, true);
            ASSERT_TRUE(loadedData.has_value()) << "Should be able to read unencrypted data with auto-detection";

            // Verify data
            ASSERT_EQ(loadedData.value().getNickname(), "UnencryptedData");
            ASSERT_EQ(loadedData.value().getHighscore(), 600);
        }
        catch (const std::exception &e)
        {
            FAIL() << "Unexpected exception: " << e.what();
        }
    }
} // namespace datacoe