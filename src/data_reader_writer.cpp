// #include "data_reader_writer.hpp"
// #include <memory>
// #include <fstream>
// #include <iostream>
// #include <stdexcept>
// #include "cryptopp/aes.h"
// #include "cryptopp/modes.h"
// #include "cryptopp/filters.h"
// #include "cryptopp/osrng.h"
// #include "cryptopp/base64.h"

// namespace DataManagement
// {
//     // Fixed Encryption Key (Warning: This is Insecure, I'm using it for learning purposes only!)
//     // Users should replace this with a secure key management system in a real-world application.
//     const CryptoPP::byte fixedKey[] = {
//         0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
//         0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
//     };

//     std::string DataReaderWriter::encrypt(const std::string &data)
//     {
//         CryptoPP::AutoSeededRandomPool rng; // random generator
//         CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE]; // initialization vector (IV)
//         rng.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE); // generate random IV
//         // creating AES Encryption object using the fixed key and the generated IV
//         // CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryption(fixedKey, CryptoPP::AES::DEFAULT_KEYLENGTH, iv);

//         std::string ciphertext;
//         try
//         {
//             // // will write the output into ciphertext
//             // CryptoPP::StringSink sink(ciphertext); 
//             // // will encrypt the data using AES and send the output into the StringSink
//             // CryptoPP::StreamTransformationFilter filter(encryption, &sink);
//             // // will take the data and send it through the StreamTransformationFilter, 
//             // // which will encrypt it with AES and send it to the StringSink and it will write the output into ciphertext
//             // CryptoPP::StringSource stringsource(data, true, &filter);

//             CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryption(fixedKey, CryptoPP::AES::DEFAULT_KEYLENGTH, iv);
//             CryptoPP::StringSink sink(ciphertext); 
//             CryptoPP::StreamTransformationFilter filter(encryption, &sink);
//             CryptoPP::StringSource stringsource(data, true, &filter);
            
//             // Option 2: If you want to keep the unique_ptr approach, fix the ownership chain:
//             /*
//             auto encryption = std::make_unique<CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption>(
//                 fixedKey, CryptoPP::AES::DEFAULT_KEYLENGTH, iv);
//             auto sink = std::make_unique<CryptoPP::StringSink>(ciphertext);
//             auto filter = std::make_unique<CryptoPP::StreamTransformationFilter>(
//                 *encryption, sink.get());
//             CryptoPP::StringSource stringsource(data, true, filter.get());
//             */
//         }
//         catch(const CryptoPP::Exception& e)
//         {
//             std::cerr << "DataReaderWriter::encrypt() Encryption Error: " << e.what() << std::endl;
//             return "";
//         }
        
//         // reinterpret the IV byte array as const char array, then create an AES::BLOCKSIZE size string out of it
//         std::string ivString(reinterpret_cast<const char*>(iv), CryptoPP::AES::BLOCKSIZE);
//         // combine the IV string (not encrypted) with the ciphertext (encrypted)
//         std::string combinedData = ivString + ciphertext;

//         std::string encoded;
//         try
//         {
//             // will write the output into encoded
//             // CryptoPP::StringSink sink(encoded);
//             // // will encode the data into Base64 binary code and send the output into the StringSink
//             // CryptoPP::Base64Encoder encoder(&sink);
//             // // will take the combinedData and send it through the Base64Encoder, 
//             // // which will encode it into Base64 binary code and send it to the StringSink,
//             // // and it will write the output into encoded
//             // CryptoPP::StringSource stringsource(combinedData, true, &encoder);

//             CryptoPP::StringSink sink(encoded);
//             CryptoPP::Base64Encoder encoder(&sink);
//             CryptoPP::StringSource stringsource(combinedData, true, &encoder);

//             /*std::unique_ptr<CryptoPP::StringSink> sink(new CryptoPP::StringSink(encoded));
//             std::unique_ptr<CryptoPP::Base64Encoder> encoder(new CryptoPP::Base64Encoder(sink.get()));
//             CryptoPP::StringSource stringsource(combinedData, true, encoder.get());*/
//         }
//         catch(const CryptoPP::Exception& e)
//         {
//             std::cerr << "DataReaderWriter::encrypt() Binary Encoding Error: " << e.what() << std::endl;
//             return "";
//         }
        
//         // if we came this far, we succeed :)
//         return encoded;
//     }
    
//     std::string DataReaderWriter::decrypt(const std::string &encodedData)
//     {
//         std::string decoded;
//         try
//         {
//             // // will write the output into decoded
//             // CryptoPP::StringSink sink(decoded);
//             // // will decode the data from Base64 binary code and send the output into the StringSink
//             // CryptoPP::Base64Decoder decoder(&sink);
//             // // will take the encodedData and send it through the Base64Decoder, 
//             // // which will decode it from Base64 binary code and send it to the StringSink,
//             // // and it will write the output into decoded
//             // CryptoPP::StringSource stringsource(encodedData, true, &decoder);
//             std::unique_ptr<CryptoPP::StringSink> sink(new CryptoPP::StringSink(decoded));
//             std::unique_ptr<CryptoPP::Base64Decoder> decoder(new CryptoPP::Base64Decoder(sink.get()));
//             CryptoPP::StringSource stringsource(encodedData, true, decoder.get());
//         }
//         catch(const CryptoPP::Exception& e)
//         {
//             std::cerr << "DataReaderWriter::decrypt() Binary Decoding Error: " << e.what() << std::endl;
//             return "";
//         }

//         if(decoded.length() <= CryptoPP::AES::BLOCKSIZE) // need to have the IV in it and more info after that
//         {
//             std::cerr << "DataReaderWriter::decrypt() Decryption Error: Invalid Encoded Data" << std::endl;
//             return "";
//         }

//         CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
//         // copy the saved IV into byte array
//         memcpy(iv, decoded.c_str(), CryptoPP::AES::BLOCKSIZE);
//         // extract only the ciphertext out of decoded
//         std::string ciphertext = decoded.substr(CryptoPP::AES::BLOCKSIZE);

//         // CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryption(fixedKey, CryptoPP::AES::DEFAULT_KEYLENGTH, iv);

//         std::string plaintext;
//         try
//         {
//             // // will write the output into plaintext
//             // CryptoPP::StringSink sink(plaintext); 
//             // // will decrypt the data using AES and send the output into the StringSink
//             // CryptoPP::StreamTransformationFilter filter(decryption, &sink);
//             // // will take the ciphertext and send it through the StreamTransformationFilter, 
//             // // which will decrypt it with AES and send it to the StringSink and it will write the output into plaintext
//             // CryptoPP::StringSource stringsource(ciphertext, true, &filter);

//             std::unique_ptr<CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption> decryption(
//                 new CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption(fixedKey, CryptoPP::AES::DEFAULT_KEYLENGTH, iv));
//             std::unique_ptr<CryptoPP::StringSink> sink(new CryptoPP::StringSink(plaintext));
//             std::unique_ptr<CryptoPP::StreamTransformationFilter> filter(new CryptoPP::StreamTransformationFilter(*decryption, sink.get()));
//             CryptoPP::StringSource stringsource(ciphertext, true, filter.get());
//         }
//         catch(const CryptoPP::Exception& e)
//         {
//             std::cerr << "DataReaderWriter::decrypt() Decryption Error: " << e.what() << std::endl;
//             return "";
//         }

//         // if we came this far, we succeed :)
//         return plaintext;
//     }

//     bool DataReaderWriter::writeData(const GameData &gamedata, const std::string &filename)
//     {
//         bool result = false;
//         std::ofstream file(filename, std::ios::binary);
//         if(!file.is_open())
//             return result;

//         std::string jsonData = gamedata.toJson().dump();

//         std::string encryptedData = encrypt(jsonData);
//         if(!encryptedData.empty()) // the encryption succeed
//         {
//             file.write(encryptedData.c_str(), encryptedData.size());
//             if(file.good())
//                 result = true;
//             else
//                 std::cerr << "DataReaderWriter::writeData() Error: File write failed." << std::endl;
//         }
//         file.close();
        
//         return result;
//     }

//     std::optional<GameData> DataReaderWriter::readData(const std::string &filename)
//     {
//         if(!std::filesystem::exists(filename))
//         {
//             std::cerr << "DataReaderWriter::readData() Error: File does not exist: " << filename << std::endl;
//             return std::nullopt;
//         }

//         std::ifstream file(filename, std::ios::binary);
//         if(!file.is_open())
//         {
//             std::cerr << "DataReaderWriter::readData() Error: Could not open the file " << filename << std::endl;
//             return std::nullopt;
//         }
//         // reading the whole file using range iterators (from begin to end, aka the whole file), and writes it into string
//         std::string encodedData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//         file.close();

//         std::string decryptedData = decrypt(encodedData);
//         try 
//         {
//             json j = json::parse(decryptedData);
//             return GameData::fromJson(j);
//         }
//         catch(const std::exception& e)
//         {
//             std::cerr << "DataReaderWriter::readData() JSON Parsing Error: " << e.what() << std::endl;
//             return std::nullopt;
//         }
//     }
// } // namespace DataManagement

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

namespace DataManagement
{
    // Fixed Encryption Key (Warning: This is Insecure, I'm using it for learning purposes only!)
    const CryptoPP::byte fixedKey[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };

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
                    new CryptoPP::StringSink(ciphertext)
                )
            );

            // Combine IV and ciphertext
            std::string ivString(reinterpret_cast<const char*>(iv), CryptoPP::AES::BLOCKSIZE);
            std::string combinedData = ivString + ciphertext;

            // Base64 encode the combined data
            std::string encoded;
            CryptoPP::StringSource ss2(combinedData, true,
                new CryptoPP::Base64Encoder(
                    new CryptoPP::StringSink(encoded)
                )
            );

            return encoded;
        }
        catch(const CryptoPP::Exception& e)
        {
            std::cerr << "DataReaderWriter::encrypt() Crypto Error: " << std::endl << e.what() << std::endl;
            return "";
        }
        catch(const std::exception& e)
        {
            std::cerr << "DataReaderWriter::encrypt() General Error: " << std::endl << e.what() << std::endl;
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
                    new CryptoPP::StringSink(decoded)
                )
            );

            // Check if decoded data has enough length for IV and ciphertext
            if(decoded.length() <= CryptoPP::AES::BLOCKSIZE)
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
                    new CryptoPP::StringSink(plaintext)
                )
            );

            return plaintext;
        }
        catch(const CryptoPP::Exception& e)
        {
            std::cerr << "DataReaderWriter::decrypt() Crypto Error: " << std::endl << e.what() << std::endl;
            return "";
        }
        catch(const std::exception& e)
        {
            std::cerr << "DataReaderWriter::decrypt() General Error: " << std::endl << e.what() << std::endl;
            return "";
        }
    }

    bool DataReaderWriter::writeData(const GameData &gamedata, const std::string &filename)
    {
        try
        {
            // Convert GameData to JSON
            std::string jsonData = gamedata.toJson().dump();
            std::cout << "Debug: GameData to JSON: " << std::endl << jsonData << std::endl;

            // Encrypt the JSON data
            std::string encryptedData = encrypt(jsonData);
            if(encryptedData.empty())
            {
                std::cerr << "DataReaderWriter::writeData() Error: Encryption failed" << std::endl;
                return false;
            }

            // Write the encrypted data to file
            std::ofstream file(filename, std::ios::binary);
            if(!file.is_open())
            {
                std::cerr << "DataReaderWriter::writeData() Error: Could not open file for writing: " << filename << std::endl;
                return false;
            }

            file.write(encryptedData.c_str(), encryptedData.size());
            if(!file.good())
            {
                std::cerr << "DataReaderWriter::writeData() Error: File write failed" << std::endl;
                file.close();
                return false;
            }

            file.close();
            return true;
        }
        catch(const std::exception& e)
        {
            std::cerr << "DataReaderWriter::writeData() Error: " << std::endl << e.what() << std::endl;
            return false;
        }
    }

    std::optional<GameData> DataReaderWriter::readData(const std::string &filename)
    {
        try
        {
            if(!std::filesystem::exists(filename))
            {
                std::cerr << "DataReaderWriter::readData() Error: File does not exist: " << filename << std::endl;
                return std::nullopt;
            }

            // Read the encrypted data from file
            std::ifstream file(filename, std::ios::binary);
            if(!file.is_open())
            {
                std::cerr << "DataReaderWriter::readData() Error: Could not open file for reading: " << filename << std::endl;
                return std::nullopt;
            }

            std::string encodedData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();

            // Decrypt the data
            std::string decryptedData = decrypt(encodedData);
            if(decryptedData.empty())
            {
                std::cerr << "DataReaderWriter::readData() Error: Decryption failed" << std::endl;
                return std::nullopt;
            }

            std::cout << "Debug: Decrypted JSON: " << std::endl << decryptedData << std::endl;

            // Parse the JSON data
            json j = json::parse(decryptedData);
            return GameData::fromJson(j);
        }
        catch(const json::exception& e)
        {
            std::cerr << "DataReaderWriter::readData() JSON Error: " << std::endl << e.what() << std::endl;
            return std::nullopt;
        }
        catch(const std::exception& e)
        {
            std::cerr << "DataReaderWriter::readData() Error: " << std::endl << e.what() << std::endl;
            return std::nullopt;
        }
    }
} // namespace DataManagement