#include "data_reader_writer.hpp"
#include <memory>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <thread>
#include <chrono>
#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/filters.h"
#include "cryptopp/osrng.h"
#include "cryptopp/base64.h"

namespace datacoe
{
    // Fixed Encryption Key (Warning: This is Insecure, I'm using it for learning purposes only!)
    const CryptoPP::byte fixedKey[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    std::string DataReaderWriter::encrypt(const std::string &data)
    {
        try
        {
            // Generate a random IV
            CryptoPP::AutoSeededRandomPool rng;
            CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
            rng.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);

            // Encrypt the data using AES in CBC mode
            std::string ciphertext;
            CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryption(fixedKey, CryptoPP::AES::DEFAULT_KEYLENGTH, iv);
            CryptoPP::StringSource ss1(data, true,
                                       new CryptoPP::StreamTransformationFilter(encryption,
                                                                                new CryptoPP::StringSink(ciphertext)));

            // Combine IV and ciphertext
            std::string ivString(reinterpret_cast<const char *>(iv), CryptoPP::AES::BLOCKSIZE);
            std::string combinedData = ivString + ciphertext;

            // Base64 encode the combined data
            std::string encoded;
            CryptoPP::StringSource ss2(combinedData, true,
                                       new CryptoPP::Base64Encoder(
                                           new CryptoPP::StringSink(encoded)));

            return encoded;
        }
        catch (const CryptoPP::Exception &e)
        {
            std::cerr << "DataReaderWriter::encrypt() Crypto Error: " << std::endl
                      << e.what() << std::endl;
            return "";
        }
        catch (const std::exception &e)
        {
            std::cerr << "DataReaderWriter::encrypt() General Error: " << std::endl
                      << e.what() << std::endl;
            return "";
        }
    }

    std::string DataReaderWriter::decrypt(const std::string &encodedData)
    {
        try
        {
            // Decode Base64
            std::string decoded;
            CryptoPP::StringSource ss1(encodedData, true,
                                       new CryptoPP::Base64Decoder(
                                           new CryptoPP::StringSink(decoded)));

            // Check if decoded data has enough length for IV and ciphertext
            if (decoded.length() <= CryptoPP::AES::BLOCKSIZE)
            {
                std::cerr << "DataReaderWriter::decrypt() Error: Decoded data too short" << std::endl;
                return "";
            }

            // Extract IV and ciphertext
            CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
            std::memcpy(iv, decoded.data(), CryptoPP::AES::BLOCKSIZE);
            std::string ciphertext = decoded.substr(CryptoPP::AES::BLOCKSIZE);

            // Decrypt the data
            std::string plaintext;
            CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryption(fixedKey, CryptoPP::AES::DEFAULT_KEYLENGTH, iv);
            CryptoPP::StringSource ss2(ciphertext, true,
                                       new CryptoPP::StreamTransformationFilter(decryption,
                                                                                new CryptoPP::StringSink(plaintext)));

            return plaintext;
        }
        catch (const CryptoPP::Exception &e)
        {
            std::cerr << "DataReaderWriter::decrypt() Crypto Error: " << std::endl
                      << e.what() << std::endl;
            return "";
        }
        catch (const std::exception &e)
        {
            std::cerr << "DataReaderWriter::decrypt() General Error: " << std::endl
                      << e.what() << std::endl;
            return "";
        }
    }

    bool DataReaderWriter::writeData(const GameData &gamedata, const std::string &filename)
    {
        try
        {
            // Convert GameData to JSON
            std::string jsonData = gamedata.toJson().dump();
            std::cout << "Debug: GameData to JSON: " << std::endl
                      << jsonData << std::endl;

            // Encrypt the JSON data
            std::string encryptedData = encrypt(jsonData);
            if (encryptedData.empty())
            {
                std::cerr << "DataReaderWriter::writeData() Error: Encryption failed" << std::endl;
                return false;
            }

            // Write the encrypted data to file
            std::ofstream file(filename, std::ios::binary);
            if (!file.is_open())
            {
                std::cerr << "DataReaderWriter::writeData() Error: Could not open file for writing: " << filename << std::endl;
                return false;
            }

            file.write(encryptedData.c_str(), encryptedData.size());
            if (!file.good())
            {
                std::cerr << "DataReaderWriter::writeData() Error: File write failed" << std::endl;
                file.close();
                return false;
            }

            file.close();
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "DataReaderWriter::writeData() Error: " << std::endl
                      << e.what() << std::endl;
            return false;
        }
    }

    std::optional<GameData> DataReaderWriter::readData(const std::string &filename)
    {
        try
        {
            if (!std::filesystem::exists(filename))
            {
                std::cerr << "DataReaderWriter::readData() Error: File does not exist: " << filename << std::endl;
                return std::nullopt;
            }

            // Read the encrypted data from file
            std::ifstream file(filename, std::ios::binary);
            if (!file.is_open())
            {
                std::cerr << "DataReaderWriter::readData() Error: Could not open file for reading: " << filename << std::endl;
                return std::nullopt;
            }

            std::string encodedData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();

            // Decrypt the data
            std::string decryptedData = decrypt(encodedData);
            if (decryptedData.empty())
            {
                std::cerr << "DataReaderWriter::readData() Error: Decryption failed" << std::endl;
                return std::nullopt;
            }

            std::cout << "Debug: Decrypted JSON: " << std::endl
                      << decryptedData << std::endl;

            // Parse the JSON data
            json j = json::parse(decryptedData);
            return GameData::fromJson(j);
        }
        catch (const json::exception &e)
        {
            std::cerr << "DataReaderWriter::readData() JSON Error: " << std::endl
                      << e.what() << std::endl;
            return std::nullopt;
        }
        catch (const std::exception &e)
        {
            std::cerr << "DataReaderWriter::readData() Error: " << std::endl
                      << e.what() << std::endl;
            return std::nullopt;
        }
    }
} // namespace datacoe