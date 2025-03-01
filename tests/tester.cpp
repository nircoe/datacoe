#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <signal.h>

// ANSI color codes for terminal output
namespace Color {
    const std::string Reset = "\033[0m";
    const std::string Red = "\033[31m";
    const std::string Green = "\033[32m";
    const std::string Yellow = "\033[33m";
    const std::string Blue = "\033[34m";
    const std::string Magenta = "\033[35m";
    const std::string Cyan = "\033[36m";
    const std::string White = "\033[37m";
    const std::string Bold = "\033[1m";
}

// Test status representation
enum class TestStatus {
    NotRun,
    Running,
    Passed,
    Failed
};

// Character representations for test states
const char NOT_RUN_CHAR = '.';
const char RUNNING_CHAR = 'R';
const char PASSED_CHAR = 'P';
const char FAILED_CHAR = 'F';
const char EMPTY_CHAR = '\0';

// Global variables for original stream buffers
std::streambuf* g_origCoutBuf = nullptr;
std::streambuf* g_origCerrBuf = nullptr;

// Custom listener for displaying test progress as a grid
class GridTestListener : public testing::TestEventListener {
private:
    testing::TestEventListener* m_originalListener;
    
    // Data for tracking test progress
    std::map<std::string, std::vector<TestStatus>> m_suiteTestStatus;
    std::map<std::string, std::vector<std::string>> m_suiteTestNames;
    std::map<std::string, std::vector<std::string>> m_failedTestOutput;
    std::map<std::string, int> m_currentTestIndex;
    std::string m_currentTestSuite;
    std::string m_currentTest;
    std::stringstream m_currentOutput;
    int m_totalTests;
    int m_completedTests;
    int m_passedTests;
    int m_failedTests;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
    
    // For stream redirection
    std::streambuf* m_origCoutBuf;
    std::streambuf* m_origCerrBuf;
    std::stringstream m_nullStream;

public:
    GridTestListener(testing::TestEventListener* listener)
        : m_originalListener(listener),
          m_totalTests(0), m_completedTests(0), m_passedTests(0), m_failedTests(0),
          m_origCoutBuf(std::cout.rdbuf()), m_origCerrBuf(std::cerr.rdbuf()) {
        m_startTime = std::chrono::high_resolution_clock::now();

        // Save global copies for signal handler
        g_origCoutBuf = m_origCoutBuf;
        g_origCerrBuf = m_origCerrBuf;

    }

    ~GridTestListener() override { 
        // Ensure we restore the original buffers
        std::cout.rdbuf(m_origCoutBuf);
        std::cerr.rdbuf(m_origCerrBuf);
        delete m_originalListener; 
    }

    void OnTestProgramStart(const testing::UnitTest& unitTest) override {
        // Redirect stdout/stderr to our null stream during test execution
        std::cout.rdbuf(m_origCoutBuf); // Temporarily restore for our output
        
        // First, gather all test suites and test cases
        m_totalTests = unitTest.total_test_count();
        for (int i = 0; i < unitTest.total_test_suite_count(); ++i) {
            const testing::TestSuite* testSuite = unitTest.GetTestSuite(i);
            std::string suiteName = testSuite->name();
            
            // Initialize vectors for this test suite
            m_suiteTestStatus[suiteName] = std::vector<TestStatus>(testSuite->total_test_count(), TestStatus::NotRun);
            m_suiteTestNames[suiteName] = std::vector<std::string>(testSuite->total_test_count());
            m_currentTestIndex[suiteName] = 0;
            
            // Save test names
            for (int j = 0; j < testSuite->total_test_count(); ++j) {
                const testing::TestInfo* testInfo = testSuite->GetTestInfo(j);
                m_suiteTestNames[suiteName][j] = testInfo->name();
            }
        }
        
        // Display header
        std::cout << Color::Bold << "Running " << m_totalTests << " tests...\n" << Color::Reset;
        printGrid();
        
        // Now redirect output
        std::cout.rdbuf(m_nullStream.rdbuf());
        std::cerr.rdbuf(m_nullStream.rdbuf());
    }

    void OnTestIterationStart(const testing::UnitTest& /*unitTest*/, int /*iteration*/) override {
        // Silent
    }

    void OnEnvironmentsSetUpStart(const testing::UnitTest& /*unitTest*/) override {
        // Silent
    }

    void OnEnvironmentsSetUpEnd(const testing::UnitTest& /*unitTest*/) override {
        // Silent
    }

    void OnTestSuiteStart(const testing::TestSuite& testSuite) override {
        m_currentTestSuite = testSuite.name();
    }

    void OnTestStart(const testing::TestInfo& testInfo) override {
        m_currentTest = testInfo.name();
        m_currentOutput.str("");
        m_currentOutput.clear();
        
        // Mark test as running
        int testIndex = m_currentTestIndex[m_currentTestSuite];
        m_suiteTestStatus[m_currentTestSuite][testIndex] = TestStatus::Running;
        
        // Update the grid
        std::cout.rdbuf(m_origCoutBuf);
        printGrid();
        std::cout.rdbuf(m_nullStream.rdbuf());
    }

    void OnTestPartResult(const testing::TestPartResult& testPartResult) override {
        if (testPartResult.failed()) {
            m_currentOutput << testPartResult.file_name() << ":" 
                            << testPartResult.line_number() << ": Failure\n"
                            << testPartResult.summary() << "\n";
        }
    }

    void OnTestEnd(const testing::TestInfo& testInfo) override {
        int testIndex = m_currentTestIndex[m_currentTestSuite]++;
        
        // Update status based on test result
        if (testInfo.result()->Failed()) {
            m_suiteTestStatus[m_currentTestSuite][testIndex] = TestStatus::Failed;
            m_failedTests++;
            
            // Store the output for failed tests
            m_failedTestOutput[m_currentTestSuite].push_back(
                m_currentTest + ":\n" + m_currentOutput.str()
            );
        } else {
            m_suiteTestStatus[m_currentTestSuite][testIndex] = TestStatus::Passed;
            m_passedTests++;
        }
        
        m_completedTests++;
        
        // Update the grid
        std::cout.rdbuf(m_origCoutBuf);
        printGrid();
        std::cout.rdbuf(m_nullStream.rdbuf());
    }

    void OnTestSuiteEnd(const testing::TestSuite& /*testSuite*/) override {
        // Silent
    }

    void OnEnvironmentsTearDownStart(const testing::UnitTest& /*unitTest*/) override {
        // Silent
    }

    void OnEnvironmentsTearDownEnd(const testing::UnitTest& /*unitTest*/) override {
        // Silent
    }

    void OnTestIterationEnd(const testing::UnitTest& /*unitTest*/, int /*iteration*/) override {
        // Silent
    }

    void OnTestProgramEnd(const testing::UnitTest& unitTest) override {
        // Restore stdout/stderr for final output
        std::cout.rdbuf(m_origCoutBuf);
        std::cerr.rdbuf(m_origCerrBuf);
        
        // Calculate elapsed time
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            endTime - m_startTime);
        
        // Print final grid state
        printGrid();
        
        // Print summary heading
        std::cout << "\n" << Color::Bold 
                  << "=========================================\n"
                  << "           Test Summary                 \n"
                  << "=========================================" << Color::Reset << "\n"
                  << "Total Tests: " << unitTest.total_test_count() << "\n"
                  << "Passed Tests: " << Color::Green << unitTest.successful_test_count() << Color::Reset << "\n"
                  << "Failed Tests: " << (unitTest.failed_test_count() > 0 ? Color::Red : "") 
                  << unitTest.failed_test_count() << Color::Reset << "\n"
                  << "Time Elapsed: " << duration.count() << " seconds\n";
        
        // Print any failed tests
        if (unitTest.failed_test_count() > 0) {
            std::cout << "\n" << Color::Bold << Color::Red
                      << "=========================================\n"
                      << "           Failed Tests                 \n"
                      << "=========================================" << Color::Reset << "\n";
            
            for (const auto& suite : m_failedTestOutput) {
                std::cout << Color::Bold << "Test Suite: " << suite.first << Color::Reset << "\n";
                for (const auto& test : suite.second) {
                    std::cout << "  " << test << "\n";
                }
            }
        }
        
        // End with a status message
        if (unitTest.failed_test_count() == 0) {
            std::cout << "\n" << Color::Green << Color::Bold 
                      << "ALL TESTS PASSED!" << Color::Reset << "\n";
        } else {
            std::cout << "\n" << Color::Red << Color::Bold
                      << unitTest.failed_test_count() 
                      << " TESTS FAILED. See details above." << Color::Reset << "\n";
        }
    }

private:
    // Print the current state of all test suites as a grid
    void printGrid() {
        // Move cursor to start of output area
        std::cout << "\033[H\033[J";  // Clear screen
        std::cout << Color::Bold << "Running " << m_totalTests << " tests... "
                  << "Completed: " << m_completedTests << "/" << m_totalTests
                  << " (P: " << m_passedTests << ", F: " << m_failedTests << ")"
                  << Color::Reset << "\n";
                  
        // Print legend
        std::cout << Color::Green << "P" << Color::Reset << " - Passed, "
                  << Color::Red << "F" << Color::Reset << " - Failed, "
                  << Color::Yellow << "R" << Color::Reset << " - Running, "
                  << ". - Not run yet\n\n";
                  
        // Print each test suite
        for (const auto& suite : m_suiteTestStatus) {
            std::string suiteName = suite.first;
            const auto& statuses = suite.second;
            
            // Count completed tests in this suite
            int suiteCompleted = 0;
            for (const auto& status : statuses) {
                if (status == TestStatus::Passed || status == TestStatus::Failed) {
                    suiteCompleted++;
                }
            }
            
            // Print suite name and status
            std::cout << std::setw(25) << std::left << suiteName << " ";
            
            // Print status of each test in the suite
            for (const auto& status : statuses) {
                char statusChar = EMPTY_CHAR;
                std::string color;
                
                switch (status) {
                    case TestStatus::NotRun:
                        statusChar = NOT_RUN_CHAR;
                        color = Color::Reset;
                        break;
                    case TestStatus::Running:
                        statusChar = RUNNING_CHAR;
                        color = Color::Yellow;
                        break;
                    case TestStatus::Passed:
                        statusChar = PASSED_CHAR;
                        color = Color::Green;
                        break;
                    case TestStatus::Failed:
                        statusChar = FAILED_CHAR;
                        color = Color::Red;
                        break;
                }
                
                std::cout << color << statusChar << Color::Reset;
            }
            
            // Print completion status for this suite
            std::cout << " (" << suiteCompleted << "/" << statuses.size() << ")\n";
        }
    }
};

void signalHandler(int signal) {
    if(g_origCoutBuf) std::cout.rdbuf(g_origCoutBuf); // Restore original cout buffer
    if(g_origCoutBuf) std::cerr.rdbuf(g_origCerrBuf); // Restore original cerr buffer
    
    std::cerr << "Test crashed with signal " << signal;
    if (signal == SIGSEGV) std::cerr << " (SIGSEGV: Segmentation fault)";
    if (signal == SIGABRT) std::cerr << " (SIGABRT: Abort)";
    std::cerr << std::endl;
    exit(128 + signal);
}

int main(int argc, char **argv) {
    // Register signal handlers
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);

    ::testing::InitGoogleTest(&argc, argv);
    
    // Remove the default listener
    auto& listeners = ::testing::UnitTest::GetInstance()->listeners();
    auto defaultListener = listeners.Release(listeners.default_result_printer());
    
    // Add our custom listener
    listeners.Append(new GridTestListener(defaultListener));
    
    return RUN_ALL_TESTS();
}