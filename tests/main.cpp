#include <testcoe.hpp>
#include <iostream>
#include <string>

int printHelp()
{
    std::cout << "Usage: ./all_tests [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help           Display this help message" << std::endl;
    std::cout << "  --all            Run all tests (default)" << std::endl;
    std::cout << "  --suite=NAME     Run only the specified test suite" << std::endl;
    std::cout << "  --test=SUITE.TEST Run only the specified test" << std::endl;
    std::cout << std::endl;
    std::cout << "Available test suites:" << std::endl;
    std::cout << "  DataManagerTest          - DataManager high-level API tests" << std::endl;
    std::cout << "  DataReaderWriterTest     - File I/O and encryption/decryption tests" << std::endl;
    std::cout << "  GameDataTest             - GameData model and serialization tests" << std::endl;
    std::cout << "  IntegrationTest          - Cross-class integration tests" << std::endl;
    std::cout << "  PerformanceTest          - Performance benchmarks" << std::endl;
    std::cout << "  MemoryTest               - Memory usage tests" << std::endl;
    std::cout << "  ErrorHandlingTest        - Error handling and recovery tests" << std::endl;
    std::cout << std::endl;
    std::cout << "Example usage:" << std::endl;
    std::cout << "  ./all_tests --suite=DataManagerTest" << std::endl;
    std::cout << "  ./all_tests --test=GameDataTest.DefaultConstructor" << std::endl;
    return 0;
}

int main(int argc, char **argv)
{
    std::cout << "====================================================" << std::endl;
    std::cout << "                 datacoe Test Suite                  " << std::endl;
    std::cout << "====================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Comprehensive testing for datacoe." << std::endl;
    std::cout << "Testing Core Module: data_manager, data_reader_writer, game_data" << std::endl;
    std::cout << "Testing Cross-Cutting: integration, performance, memory, error handling" << std::endl;
    std::cout << std::endl;

    bool askForAll = false;
    std::string suiteName;
    std::string testName;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--help")
            return printHelp();
        else if (arg == "--all")
            askForAll = true;
        else if (!askForAll && arg.substr(0, 8) == "--suite=")
        {
            suiteName = arg.substr(8);
            if (suiteName.empty())
            {
                std::cerr << "Error: --suite= requires a suite name" << std::endl;
                return 1;
            }
        }
        else if (!askForAll && arg.substr(0, 7) == "--test=")
        {
            std::string fullTest = arg.substr(7);
            size_t dotPos = fullTest.find('.');
            if (dotPos == std::string::npos)
            {
                std::cerr << "Error: --test= requires SUITE.TEST" << std::endl;
                return 1;
            }
            suiteName = fullTest.substr(0, dotPos);
            testName = fullTest.substr(dotPos + 1);
        }
    }

    testcoe::init(&argc, argv);

    if (askForAll || (testName.empty() && suiteName.empty()))
    {
        std::cout << "Running all datacoe tests..." << std::endl;
        return testcoe::run();
    }

    if (!testName.empty())
    {
        std::cout << "Running test: " << suiteName << "." << testName << std::endl;
        return testcoe::run_test(suiteName, testName);
    }

    if (!suiteName.empty())
    {
        std::cout << "Running suite: " << suiteName << std::endl;
        return testcoe::run_suite(suiteName);
    }

    return 0;
}
