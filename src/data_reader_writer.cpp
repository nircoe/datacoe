#include "data_reader_writer.hpp"
#include <memory>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <thread>
#include <chrono>
#include <vector>
#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/filters.h"
#include "cryptopp/osrng.h"
#include "cryptopp/base64.h"

namespace datacoe
{
    const std::string ENCRYPTION_PREFIX = "DATACOE_ENCRYPTED";

    // Fixed Encryption Key (Warning: This is Insecure, I'm using it for learning purposes only!)
    const CryptoPP::byte fixedKey[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    bool DataReaderWriter::isFileEncrypted(const std::string &filename)
    {
        if (!std::filesystem::exists(filename))
            return false;

        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open())
            return false;

        // Read just enough bytes to check for our prefix
        std::vector<char> header(ENCRYPTION_PREFIX.size());
        file.read(header.data(), ENCRYPTION_PREFIX.size());

        // Check if we read enough bytes
        if (file.gcount() < static_cast<std::streamsize>(ENCRYPTION_PREFIX.size()))
            return false;

        // Compare with our prefix
        return std::string(header.data(), static_cast<size_t>(file.gcount())) == ENCRYPTION_PREFIX;
    }

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

            return ENCRYPTION_PREFIX + encoded;
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
            std::string dataToDecrypt = encodedData;
            if (dataToDecrypt.substr(0, ENCRYPTION_PREFIX.size()) == ENCRYPTION_PREFIX)
            {
                dataToDecrypt = dataToDecrypt.substr(ENCRYPTION_PREFIX.size());
            }
            else
            {
                std::cerr << "DataReaderWriter::decrypt() Warning: Missing encryption prefix" << std::endl;
                // Continue anyway in case it's an older file without the prefix
            }

            // Decode Base64
            std::string decoded;
            CryptoPP::StringSource ss1(dataToDecrypt, true,
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

    bool DataReaderWriter::writeData(const GameData &gamedata, const std::string &filename, bool encryption)
    {
        try
        {
            // Convert GameData to JSON
            std::string jsonData = gamedata.toJson().dump();
            std::cout << "Debug: GameData to JSON: " << std::endl
                      << jsonData << std::endl;

            std::string writeableData;

            if(encryption)
            {
                // Encrypt the JSON data
                std::string encryptedData = encrypt(jsonData);
                if (encryptedData.empty())
                {
                    std::cerr << "DataReaderWriter::writeData() Error: Encryption failed" << std::endl;
                    return false;
                }
                writeableData = encryptedData;
            }
            else // no encryption
                writeableData = jsonData;

            // Write the data to file
            auto openmode = encryption ? std::ios::binary : std::ios::out;
            std::ofstream file(filename, openmode);
            if (!file.is_open())
            {
                std::cerr << "DataReaderWriter::writeData() Error: Could not open file for writing: " << filename << std::endl;
                return false;
            }

            file.write(writeableData.c_str(), writeableData.size());
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

    std::optional<GameData> DataReaderWriter::readData(const std::string &filename, bool decryption)
    {
        try
        {
            if (!std::filesystem::exists(filename))
            {
                std::cerr << "DataReaderWriter::readData() Error: File does not exist: " << filename << std::endl;
                return std::nullopt;
            }

            bool fileIsEncrypted = isFileEncrypted(filename);
            if(fileIsEncrypted != decryption)
            {
                std::cerr << "DataReaderWriter::readData() Warning: "
                          << (fileIsEncrypted ? "File is encrypted but decryption=false" 
                                              : "File is not encrypted but decryption=true")
                          << " - Adjusting decryption flag to match file state" << std::endl;
                decryption = fileIsEncrypted;
            }

            // Read the data from file
            auto openmode = decryption ? std::ios::binary : std::ios::in;
            std::ifstream file(filename, openmode);
            if (!file.is_open())
            {
                std::cerr << "DataReaderWriter::readData() Error: Could not open file for reading: " << filename << std::endl;
                return std::nullopt;
            }

            std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();

            std::string parseableData;

            if(decryption)
            {
                // Decrypt the data
                std::string decryptedData = decrypt(data);
                if (decryptedData.empty())
                {
                    std::cerr << "DataReaderWriter::readData() Error: Decryption failed" << std::endl;
                    return std::nullopt;
                }

                std::cout << "Debug: Decrypted JSON: " << std::endl
                        << decryptedData << std::endl;
                parseableData = decryptedData;
            }
            else // no decryption
                parseableData = data;

            // Parse the JSON data
            json j = json::parse(parseableData);
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