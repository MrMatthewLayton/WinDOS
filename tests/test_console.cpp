#include "test_framework.hpp"

using namespace System;

void TestBasicOutput() {
    Test::PrintHeader("Basic Output");

    // These tests verify the output functions work without crashing
    // Visual verification is needed in DOSBox

    Console::Write("Testing Write without newline... ");
    Console::WriteLine("OK");
    Test::Pass("Write and WriteLine work");

    Console::WriteLine("Testing empty WriteLine:");
    Console::WriteLine();
    Console::WriteLine("Above should be blank line");
    Test::Pass("Empty WriteLine works");

    Console::Write("Tab test:\tafter tab");
    Console::WriteLine();
    Test::Pass("Tab character works");

    Test::PrintSummary();
}

void TestTypeOutput() {
    Test::PrintHeader("Type Output");

    Console::Write("Int32: ");
    Console::WriteLine(Int32(12345));
    Test::Pass("Int32 output");

    Console::Write("Negative Int32: ");
    Console::WriteLine(Int32(-9876));
    Test::Pass("Negative Int32 output");

    Console::Write("UInt32: ");
    Console::WriteLine(UInt32(4000000000u));
    Test::Pass("UInt32 output");

    Console::Write("Int64: ");
    Console::WriteLine(Int64(1234567890123LL));
    Test::Pass("Int64 output");

    Console::Write("Float32: ");
    Console::WriteLine(Float32(3.14159f));
    Test::Pass("Float32 output");

    Console::Write("Float64: ");
    Console::WriteLine(Float64(2.718281828));
    Test::Pass("Float64 output");

    Console::Write("Boolean true: ");
    Console::WriteLine(Boolean(true));
    Test::Pass("Boolean true output");

    Console::Write("Boolean false: ");
    Console::WriteLine(Boolean(false));
    Test::Pass("Boolean false output");

    Console::Write("Char: ");
    Console::WriteLine(Char('X'));
    Test::Pass("Char output");

    Console::Write("String: ");
    Console::WriteLine(String("Hello World"));
    Test::Pass("String output");

    Test::PrintSummary();
}

void TestColors() {
    Test::PrintHeader("Colors");

    Console::WriteLine("Testing all 16 colors:");

    const char* colorNames[] = {
        "Black", "DarkBlue", "DarkGreen", "DarkCyan",
        "DarkRed", "DarkMagenta", "DarkYellow", "Gray",
        "DarkGray", "Blue", "Green", "Cyan",
        "Red", "Magenta", "Yellow", "White"
    };

    for (int i = 0; i < 16; i++) {
        Console::SetForegroundColor(static_cast<ConsoleColor>(i));
        if (i == 0) {
            // Black on black won't show, use different background
            Console::SetBackgroundColor(ConsoleColor::Gray);
        } else {
            Console::SetBackgroundColor(ConsoleColor::Black);
        }
        Console::Write(colorNames[i]);
        Console::Write(" ");
    }
    Console::ResetColor();
    Console::WriteLine();

    Test::Pass("All foreground colors displayed");

    Console::WriteLine("Background colors:");
    for (int i = 0; i < 8; i++) {
        Console::SetBackgroundColor(static_cast<ConsoleColor>(i));
        Console::SetForegroundColor(i == 0 ? ConsoleColor::White : ConsoleColor::Black);
        Console::Write(" ");
        Console::Write(colorNames[i]);
        Console::Write(" ");
    }
    Console::ResetColor();
    Console::WriteLine();

    Test::Pass("Background colors displayed");

    // Test color get/set
    Console::SetForegroundColor(ConsoleColor::Yellow);
    ASSERT(Console::ForegroundColor() == ConsoleColor::Yellow, "ForegroundColor getter");

    Console::SetBackgroundColor(ConsoleColor::Blue);
    ASSERT(Console::BackgroundColor() == ConsoleColor::Blue, "BackgroundColor getter");

    Console::ResetColor();
    ASSERT(Console::ForegroundColor() == ConsoleColor::Gray, "ResetColor resets foreground");
    ASSERT(Console::BackgroundColor() == ConsoleColor::Black, "ResetColor resets background");

    Test::PrintSummary();
}

void TestCursorPosition() {
    Test::PrintHeader("Cursor Position");

    // Save current position (unused but may be useful for restoring)
    (void)Console::CursorLeft();
    int startTop = static_cast<int>(Console::CursorTop());

    // Move cursor and write
    Console::SetCursorPosition(40, 15);
    Console::Write("@");

    // Check position
    int newLeft = static_cast<int>(Console::CursorLeft());
    int newTop = static_cast<int>(Console::CursorTop());
    ASSERT(newLeft == 41, "CursorLeft after write");
    ASSERT(newTop == 15, "CursorTop after SetCursorPosition");

    // Move back
    Console::SetCursorPosition(0, startTop + 1);
    Test::Pass("SetCursorPosition works");

    // Test bounds
    Console::SetCursorPosition(-1, -1);  // Should clamp to 0,0
    ASSERT(static_cast<int>(Console::CursorLeft()) == 0, "Negative position clamped to 0");
    ASSERT(static_cast<int>(Console::CursorTop()) == 0, "Negative position clamped to 0");

    // Move to reasonable position for rest of tests
    Console::SetCursorPosition(0, startTop + 2);

    Test::PrintSummary();
}

void TestWindowSize() {
    Test::PrintHeader("Window Size");

    int width = static_cast<int>(Console::WindowWidth());
    int height = static_cast<int>(Console::WindowHeight());

    Console::Write("Window size: ");
    Console::Write(width);
    Console::Write("x");
    Console::WriteLine(height);

    ASSERT(width > 0, "WindowWidth is positive");
    ASSERT(height > 0, "WindowHeight is positive");
    ASSERT(width >= 40, "WindowWidth at least 40");  // Standard minimum
    ASSERT(height >= 20, "WindowHeight at least 20"); // Standard minimum

    Test::PrintSummary();
}

void TestClear() {
    Test::PrintHeader("Clear Screen");

    Console::WriteLine("About to clear screen in 2 seconds...");
    Console::WriteLine("You should see a clean screen, then this test header.");

    // Small delay simulation (not actual delay)
    for (volatile int i = 0; i < 10000000; i++) {}

    Console::Clear();

    Console::SetForegroundColor(ConsoleColor::Green);
    Console::WriteLine("Screen was cleared!");
    Console::ResetColor();
    Console::WriteLine();

    // Verify cursor is at 0,0 after clear
    ASSERT(static_cast<int>(Console::CursorTop()) == 1, "Cursor at top after clear + 1 line");

    Test::Pass("Clear function executed");

    Test::PrintSummary();
}

void TestColoredBox() {
    Test::PrintHeader("Colored Box Demo");

    Console::WriteLine("Drawing a colored box:");
    Console::WriteLine();

    // Draw a simple colored box
    Console::SetBackgroundColor(ConsoleColor::Blue);
    Console::SetForegroundColor(ConsoleColor::Yellow);

    for (int row = 0; row < 5; row++) {
        Console::Write("  ");
        for (int col = 0; col < 20; col++) {
            if (row == 0 || row == 4) {
                Console::Write("=");
            } else if (col == 0 || col == 19) {
                Console::Write("|");
            } else if (row == 2 && col >= 4 && col < 16) {
                const char* text = " BCL Test! ";
                int idx = col - 4;
                if (idx < 11) {
                    Console::Write(text[idx]);
                } else {
                    Console::Write(" ");
                }
            } else {
                Console::Write(" ");
            }
        }
        Console::Write("  ");
        Console::ResetColor();
        Console::WriteLine();
        Console::SetBackgroundColor(ConsoleColor::Blue);
        Console::SetForegroundColor(ConsoleColor::Yellow);
    }

    Console::ResetColor();
    Console::WriteLine();

    Test::Pass("Colored box rendered");

    Test::PrintSummary();
}

void TestSpecialCharacters() {
    Test::PrintHeader("Special Characters");

    Console::WriteLine("Backspace test: ABC\b\b_");
    Test::Pass("Backspace character");

    Console::Write("Carriage return test: XXXXX\rOK");
    Console::WriteLine();
    Test::Pass("Carriage return");

    Console::WriteLine("Extended ASCII box characters:");
    Console::WriteLine("+---------+");
    Console::WriteLine("|  Box    |");
    Console::WriteLine("+---------+");
    Test::Pass("Box drawing characters");

    Test::PrintSummary();
}

int main() {
    Console::Clear();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("BCL Console Test Suite");
    Console::WriteLine("======================");
    Console::ResetColor();
    Console::WriteLine();
    Console::WriteLine("Note: Some tests require visual verification.");
    Console::WriteLine();

    TestBasicOutput();
    TestTypeOutput();
    TestColors();
    TestCursorPosition();
    TestWindowSize();
    TestSpecialCharacters();
    TestColoredBox();

    // Don't run TestClear last as it clears the results
    // TestClear();

    Console::WriteLine();
    Console::SetForegroundColor(ConsoleColor::White);
    Console::WriteLine("All console tests complete!");
    Console::ResetColor();

    Console::WriteLine();
    Console::Write("Press any key to exit...");
    Console::ReadKey(true);

    return 0;
}
