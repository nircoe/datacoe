#include <testcoe.hpp>
#include <cstddef>
#include <iostream>
#include <string>

int print_help()
{
    std::cout << "Usage: ./datacoe_tests [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help           Display this help message" << std::endl;
    std::cout << "  --all            Run all tests (default)" << std::endl;
    std::cout << "  --suite=NAME     Run only the specified test suite" << std::endl;
    std::cout << "  --test=SUITE.TEST Run only the specified test" << std::endl;
    std::cout << std::endl;
    std::cout << "Available test suites:" << std::endl;
    std::cout << "  SaveableTests            - Saveable concept tests" << std::endl;
    std::cout << "  IoTests                  - Internal file I/O and encryption/decryption tests" << std::endl;
    std::cout << "  SaveLoadTests            - Save and load API tests" << std::endl;
    std::cout << "  IntegrationTests         - Save and load workflow tests" << std::endl;
    std::cout << "  ErrorHandlingTests       - Error handling and recovery tests" << std::endl;
    std::cout << "  MemoryTests              - Memory usage tests" << std::endl;
    std::cout << "  PerformanceTests         - Performance benchmarks" << std::endl;
    std::cout << std::endl;
    std::cout << "Example usage:" << std::endl;
    std::cout << "  ./datacoe_tests --suite=SaveLoadTests" << std::endl;
    std::cout << "  ./datacoe_tests --test=SaveLoadTests.PlainRoundTrip" << std::endl;
    return 0;
}

int main(int argc, char **argv)
{
    std::cout << "====================================================" << std::endl;
    std::cout << "                 datacoe Test Suite                  " << std::endl;
    std::cout << "====================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Comprehensive testing for datacoe." << std::endl;
    std::cout << "Testing Core Module: saveable, io, save_load" << std::endl;
    std::cout << "Testing Cross-Cutting: integration, error handling, memory, performance" << std::endl;
    std::cout << std::endl;

    bool ask_for_all = false;
    std::string suite_name;
    std::string test_name;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--help")
            return print_help();
        else if (arg == "--all")
            ask_for_all = true;
        else if (!ask_for_all && arg.substr(0, 8) == "--suite=")
        {
            suite_name = arg.substr(8);
            if (suite_name.empty())
            {
                std::cerr << "Error: --suite= requires a suite name" << std::endl;
                return 1;
            }
        }
        else if (!ask_for_all && arg.substr(0, 7) == "--test=")
        {
            std::string full_test = arg.substr(7);
            std::size_t dot_pos = full_test.find('.');
            if (dot_pos == std::string::npos)
            {
                std::cerr << "Error: --test= requires SUITE.TEST" << std::endl;
                return 1;
            }
            suite_name = full_test.substr(0, dot_pos);
            test_name = full_test.substr(dot_pos + 1);
        }
    }

    testcoe::init(&argc, argv);

    if (ask_for_all || (test_name.empty() && suite_name.empty()))
    {
        std::cout << "Running all datacoe tests..." << std::endl;
        return testcoe::run();
    }

    if (!test_name.empty())
    {
        std::cout << "Running test: " << suite_name << "." << test_name << std::endl;
        return testcoe::run_test(suite_name, test_name);
    }

    if (!suite_name.empty())
    {
        std::cout << "Running suite: " << suite_name << std::endl;
        return testcoe::run_suite(suite_name);
    }

    return 0;
}
