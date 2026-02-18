#ifndef TEST_FRAMEWORK_HPP
#define TEST_FRAMEWORK_HPP

#include "../src/rtcorlib.hpp"

// Simple test framework for DOS environment

namespace Test {

static int _passCount = 0;
static int _failCount = 0;

inline void ResetCounts() {
    _passCount = 0;
    _failCount = 0;
}

inline void PrintHeader(const char* testName) {
    System::Console::WriteLine();
    System::Console::SetForegroundColor(System::ConsoleColor::White);
    System::Console::Write("=== Testing ");
    System::Console::Write(testName);
    System::Console::WriteLine(" ===");
    System::Console::ResetColor();
    ResetCounts();
}

inline void Pass(const char* message) {
    _passCount++;
    System::Console::SetForegroundColor(System::ConsoleColor::Green);
    System::Console::Write("[PASS] ");
    System::Console::ResetColor();
    System::Console::WriteLine(message);
}

inline void Fail(const char* message) {
    _failCount++;
    System::Console::SetForegroundColor(System::ConsoleColor::Red);
    System::Console::Write("[FAIL] ");
    System::Console::ResetColor();
    System::Console::WriteLine(message);
}

inline void PrintSummary() {
    System::Console::WriteLine();
    int total = _passCount + _failCount;

    if (_failCount == 0) {
        System::Console::SetForegroundColor(System::ConsoleColor::Green);
    } else {
        System::Console::SetForegroundColor(System::ConsoleColor::Yellow);
    }

    System::Console::Write("Results: ");
    System::Console::Write(_passCount);
    System::Console::Write("/");
    System::Console::Write(total);
    System::Console::WriteLine(" passed");
    System::Console::ResetColor();
}

inline bool AllPassed() {
    return _failCount == 0;
}

} // namespace Test

// Macros for common test patterns
#define ASSERT(condition, message) \
    do { \
        if (condition) { \
            Test::Pass(message); \
        } else { \
            Test::Fail(message); \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual, message) \
    do { \
        if ((expected) == (actual)) { \
            Test::Pass(message); \
        } else { \
            Test::Fail(message); \
        } \
    } while(0)

#define ASSERT_NE(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            Test::Pass(message); \
        } else { \
            Test::Fail(message); \
        } \
    } while(0)

#define ASSERT_THROWS(expr, ExceptionType, message) \
    do { \
        bool caught = false; \
        try { \
            expr; \
        } catch (const ExceptionType&) { \
            caught = true; \
        } catch (...) { \
        } \
        if (caught) { \
            Test::Pass(message); \
        } else { \
            Test::Fail(message); \
        } \
    } while(0)

#define ASSERT_NO_THROW(expr, message) \
    do { \
        bool threw = false; \
        try { \
            expr; \
        } catch (...) { \
            threw = true; \
        } \
        if (!threw) { \
            Test::Pass(message); \
        } else { \
            Test::Fail(message); \
        } \
    } while(0)

#endif // TEST_FRAMEWORK_HPP
