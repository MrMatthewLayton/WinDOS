#include "test_framework.hpp"
#include <cstring>

using namespace System;

void TestBaseException() {
    Test::PrintHeader("Base Exception");

    Exception e1;
    ASSERT(std::strlen(e1.Message()) > 0, "Default exception has message");
    ASSERT(std::strlen(e1.what()) > 0, "what() returns message");

    Exception e2("Custom error message");
    ASSERT(std::strcmp(e2.Message(), "Custom error message") == 0, "Custom message stored");

    // Copy constructor
    Exception e3(e2);
    ASSERT(std::strcmp(e3.Message(), "Custom error message") == 0, "Copy constructor copies message");

    // Copy assignment
    Exception e4;
    e4 = e2;
    ASSERT(std::strcmp(e4.Message(), "Custom error message") == 0, "Copy assignment copies message");

    Test::PrintSummary();
}

void TestArgumentException() {
    Test::PrintHeader("ArgumentException");

    ArgumentException e1("Value is invalid", "paramName");
    ASSERT(std::strcmp(e1.Message(), "Value is invalid") == 0, "Message stored");
    ASSERT(std::strcmp(e1.ParamName(), "paramName") == 0, "ParamName stored");

    ArgumentException e2("Message only");
    ASSERT(std::strlen(e2.ParamName()) == 0, "ParamName empty when not provided");

    // Copy
    ArgumentException e3(e1);
    ASSERT(std::strcmp(e3.ParamName(), "paramName") == 0, "Copy preserves ParamName");

    Test::PrintSummary();
}

void TestArgumentNullException() {
    Test::PrintHeader("ArgumentNullException");

    ArgumentNullException e("myParam");
    ASSERT(std::strcmp(e.ParamName(), "myParam") == 0, "ParamName stored");
    ASSERT(std::strstr(e.Message(), "null") != nullptr, "Message mentions null");

    Test::PrintSummary();
}

void TestArgumentOutOfRangeException() {
    Test::PrintHeader("ArgumentOutOfRangeException");

    ArgumentOutOfRangeException e1("index");
    ASSERT(std::strcmp(e1.ParamName(), "index") == 0, "ParamName stored");
    ASSERT(std::strstr(e1.Message(), "range") != nullptr, "Default message mentions range");

    ArgumentOutOfRangeException e2("index", "Must be positive");
    ASSERT(std::strcmp(e2.Message(), "Must be positive") == 0, "Custom message stored");

    Test::PrintSummary();
}

void TestInvalidOperationException() {
    Test::PrintHeader("InvalidOperationException");

    InvalidOperationException e("Operation not allowed");
    ASSERT(std::strcmp(e.Message(), "Operation not allowed") == 0, "Message stored");

    Test::PrintSummary();
}

void TestIndexOutOfRangeException() {
    Test::PrintHeader("IndexOutOfRangeException");

    IndexOutOfRangeException e1;
    ASSERT(std::strstr(e1.Message(), "bounds") != nullptr ||
           std::strstr(e1.Message(), "Index") != nullptr, "Default message about bounds");

    IndexOutOfRangeException e2("Index -1 is invalid");
    ASSERT(std::strcmp(e2.Message(), "Index -1 is invalid") == 0, "Custom message stored");

    Test::PrintSummary();
}

void TestNullReferenceException() {
    Test::PrintHeader("NullReferenceException");

    NullReferenceException e1;
    ASSERT(std::strstr(e1.Message(), "null") != nullptr ||
           std::strstr(e1.Message(), "Object") != nullptr, "Default message about null");

    NullReferenceException e2("Pointer was null");
    ASSERT(std::strcmp(e2.Message(), "Pointer was null") == 0, "Custom message stored");

    Test::PrintSummary();
}

void TestFormatException() {
    Test::PrintHeader("FormatException");

    FormatException e1;
    ASSERT(std::strstr(e1.Message(), "format") != nullptr, "Default message about format");

    FormatException e2("Invalid number format");
    ASSERT(std::strcmp(e2.Message(), "Invalid number format") == 0, "Custom message stored");

    Test::PrintSummary();
}

void TestOverflowException() {
    Test::PrintHeader("OverflowException");

    OverflowException e1;
    ASSERT(std::strstr(e1.Message(), "overflow") != nullptr ||
           std::strstr(e1.Message(), "Overflow") != nullptr, "Default message about overflow");

    OverflowException e2("Value too large");
    ASSERT(std::strcmp(e2.Message(), "Value too large") == 0, "Custom message stored");

    Test::PrintSummary();
}

void TestThrowCatch() {
    Test::PrintHeader("Throw and Catch");

    // Test throwing and catching base exception
    bool caught = false;
    try {
        throw Exception("Test exception");
    } catch (const Exception& e) {
        caught = true;
        ASSERT(std::strcmp(e.Message(), "Test exception") == 0, "Caught exception has correct message");
    }
    ASSERT(caught, "Exception was caught");

    // Test polymorphic catch
    caught = false;
    try {
        throw IndexOutOfRangeException("Array access error");
    } catch (const Exception& e) {
        caught = true;
        ASSERT(std::strcmp(e.Message(), "Array access error") == 0, "Derived exception caught as base");
    }
    ASSERT(caught, "Derived exception caught as base");

    // Test catching specific type
    caught = false;
    bool wrongType = false;
    try {
        throw ArgumentException("Bad argument", "param");
    } catch (const IndexOutOfRangeException&) {
        wrongType = true;
    } catch (const ArgumentException& e) {
        caught = true;
        ASSERT(std::strcmp(e.ParamName(), "param") == 0, "Specific exception type caught");
    } catch (...) {
        wrongType = true;
    }
    ASSERT(caught && !wrongType, "Correct exception type caught");

    // Test rethrowing
    caught = false;
    try {
        try {
            throw InvalidOperationException("Cannot do that");
        } catch (const Exception&) {
            throw;  // Rethrow
        }
    } catch (const InvalidOperationException& e) {
        caught = true;
        ASSERT(std::strcmp(e.Message(), "Cannot do that") == 0, "Rethrown exception preserved");
    }
    ASSERT(caught, "Rethrown exception caught");

    Test::PrintSummary();
}

void TestExceptionInBCL() {
    Test::PrintHeader("Exceptions from BCL");

    // Array bounds
    bool caught = false;
    try {
        Array<Int32> arr(5);
        int x = arr[10];  // Should throw
        (void)x;
    } catch (const IndexOutOfRangeException&) {
        caught = true;
    }
    ASSERT(caught, "Array bounds checking throws IndexOutOfRangeException");

    // String bounds
    caught = false;
    try {
        String s("Hello");
        char c = s[100];  // Should throw
        (void)c;
    } catch (const IndexOutOfRangeException&) {
        caught = true;
    }
    ASSERT(caught, "String bounds checking throws IndexOutOfRangeException");

    // Division by zero
    caught = false;
    try {
        Int32 a(10);
        Int32 b(0);
        Int32 c = a / b;  // Should throw
        (void)c;
    } catch (const InvalidOperationException&) {
        caught = true;
    }
    ASSERT(caught, "Division by zero throws InvalidOperationException");

    // Parse failure
    caught = false;
    try {
        Int32 x = Int32::Parse("not a number");
        (void)x;
    } catch (const FormatException&) {
        caught = true;
    }
    ASSERT(caught, "Parse failure throws FormatException");

    // Substring out of range
    caught = false;
    try {
        String s("Hello");
        String sub = s.Substring(-1);
        (void)sub;
    } catch (const ArgumentOutOfRangeException&) {
        caught = true;
    }
    ASSERT(caught, "Substring with negative index throws ArgumentOutOfRangeException");

    Test::PrintSummary();
}

int main() {
    Console::Clear();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("BCL Exception Test Suite");
    Console::WriteLine("========================");
    Console::ResetColor();

    TestBaseException();
    TestArgumentException();
    TestArgumentNullException();
    TestArgumentOutOfRangeException();
    TestInvalidOperationException();
    TestIndexOutOfRangeException();
    TestNullReferenceException();
    TestFormatException();
    TestOverflowException();
    TestThrowCatch();
    TestExceptionInBCL();

    Console::WriteLine();
    Console::SetForegroundColor(ConsoleColor::White);
    Console::WriteLine("All exception tests complete!");
    Console::ResetColor();

    Console::WriteLine();
    Console::Write("Press any key to exit...");
    Console::ReadKey(true);

    return 0;
}
