// This file provides a simple menu to run individual test suites
// In a real scenario, you would compile each test separately

#include "../src/rtcorlib.hpp"

using namespace System;

void PrintMenu() {
    Console::Clear();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("=================================");
    Console::WriteLine("    rtcorlib Test Suite Runner");
    Console::WriteLine("=================================");
    Console::ResetColor();
    Console::WriteLine();

    Console::WriteLine("Available test programs:");
    Console::WriteLine();

    Console::SetForegroundColor(ConsoleColor::White);
    Console::Write("  1. ");
    Console::ResetColor();
    Console::WriteLine("test_types.exe    - Primitive wrapper types");

    Console::SetForegroundColor(ConsoleColor::White);
    Console::Write("  2. ");
    Console::ResetColor();
    Console::WriteLine("test_string.exe   - String class");

    Console::SetForegroundColor(ConsoleColor::White);
    Console::Write("  3. ");
    Console::ResetColor();
    Console::WriteLine("test_array.exe    - Array<T> template");

    Console::SetForegroundColor(ConsoleColor::White);
    Console::Write("  4. ");
    Console::ResetColor();
    Console::WriteLine("test_exception.exe - Exception hierarchy");

    Console::SetForegroundColor(ConsoleColor::White);
    Console::Write("  5. ");
    Console::ResetColor();
    Console::WriteLine("test_console.exe  - Console I/O");

    Console::WriteLine();
    Console::SetForegroundColor(ConsoleColor::Yellow);
    Console::WriteLine("Run each test individually from DOS prompt:");
    Console::ResetColor();
    Console::WriteLine("  C:\\TESTS> test_types.exe");
    Console::WriteLine();

    Console::SetForegroundColor(ConsoleColor::Green);
    Console::WriteLine("Build all tests with: make tests");
    Console::ResetColor();
    Console::WriteLine();
}

void DemoBasicFunctionality() {
    Console::WriteLine();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("=== Quick rtcorlib Demo ===");
    Console::ResetColor();
    Console::WriteLine();

    // Types demo
    Console::Write("Int32 value: ");
    Int32 num = 42;
    Console::WriteLine(num.ToString());

    Console::Write("Int32 + 10 = ");
    Console::WriteLine((num + Int32(10)).ToString());

    // String demo
    Console::WriteLine();
    String s1("Hello");
    String s2(" World!");
    String combined = s1 + s2;
    Console::Write("String concatenation: ");
    Console::WriteLine(combined);

    Console::Write("ToUpper: ");
    Console::WriteLine(combined.ToUpper());

    Console::Write("Substring(0,5): ");
    Console::WriteLine(combined.Substring(0, 5));

    // Array demo
    Console::WriteLine();
    Array<Int32> arr = {10, 20, 30, 40, 50};
    Console::Write("Array: ");
    for (int i = 0; i < arr.Length(); i++) {
        Console::Write(arr[i].ToString());
        if (i < arr.Length() - 1) Console::Write(", ");
    }
    Console::WriteLine();

    arr.Reverse();
    Console::Write("Reversed: ");
    for (int i = 0; i < arr.Length(); i++) {
        Console::Write(arr[i].ToString());
        if (i < arr.Length() - 1) Console::Write(", ");
    }
    Console::WriteLine();

    // Exception demo
    Console::WriteLine();
    Console::WriteLine("Exception handling demo:");
    try {
        Console::WriteLine("  Attempting arr[10]...");
        Int32 x = arr[10];
        (void)x;
    } catch (const IndexOutOfRangeException& e) {
        Console::SetForegroundColor(ConsoleColor::Red);
        Console::Write("  Caught: ");
        Console::WriteLine(e.Message());
        Console::ResetColor();
    }

    // Color demo
    Console::WriteLine();
    Console::WriteLine("Color demo:");
    Console::Write("  ");
    for (int i = 0; i < 16; i++) {
        Console::SetForegroundColor(static_cast<ConsoleColor>(i));
        if (i == 0) Console::SetBackgroundColor(ConsoleColor::Gray);
        Console::Write("*");
        Console::SetBackgroundColor(ConsoleColor::Black);
    }
    Console::ResetColor();
    Console::WriteLine();

    Console::WriteLine();
}

int main() {
    PrintMenu();
    DemoBasicFunctionality();

    Console::SetForegroundColor(ConsoleColor::White);
    Console::WriteLine("Press any key to exit...");
    Console::ResetColor();
    Console::ReadKey(true);

    return 0;
}
