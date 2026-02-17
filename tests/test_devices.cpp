// Device tests for BCL
// Tests MouseStatus, KeyboardStatus structures and Display class
// Note: Hardware-dependent methods (Initialize, ReadKey, etc.) require actual hardware

#include "test_framework.hpp"
#include "../bcl/System/Devices.hpp"

using namespace System;
using namespace System::Devices;

void TestMouseStatus() {
    Test::PrintHeader("MouseStatus");

    // Default constructor
    MouseStatus ms1;
    ASSERT_EQ(0, ms1.x, "Default x is 0");
    ASSERT_EQ(0, ms1.y, "Default y is 0");
    ASSERT(!ms1.leftButton, "Default leftButton is false");
    ASSERT(!ms1.rightButton, "Default rightButton is false");
    ASSERT(!ms1.middleButton, "Default middleButton is false");

    // Parameterized constructor
    MouseStatus ms2(100, 200, true, false, true);
    ASSERT_EQ(100, ms2.x, "Parameterized x");
    ASSERT_EQ(200, ms2.y, "Parameterized y");
    ASSERT(ms2.leftButton, "Parameterized leftButton");
    ASSERT(!ms2.rightButton, "Parameterized rightButton");
    ASSERT(ms2.middleButton, "Parameterized middleButton");

    // Copy
    MouseStatus ms3 = ms2;
    ASSERT_EQ(100, ms3.x, "Copy x");
    ASSERT(ms3.leftButton, "Copy leftButton");

    // Modify
    ms3.x = 50;
    ms3.leftButton = false;
    ASSERT_EQ(50, ms3.x, "Modified x");
    ASSERT(!ms3.leftButton, "Modified leftButton");
    ASSERT_EQ(100, ms2.x, "Original unchanged");

    Test::PrintSummary();
}

void TestKeyboardStatus() {
    Test::PrintHeader("KeyboardStatus");

    // Default constructor
    KeyboardStatus ks1;
    ASSERT(!ks1.shiftPressed, "Default shiftPressed is false");
    ASSERT(!ks1.ctrlPressed, "Default ctrlPressed is false");
    ASSERT(!ks1.altPressed, "Default altPressed is false");
    ASSERT(!ks1.capsLock, "Default capsLock is false");
    ASSERT(!ks1.numLock, "Default numLock is false");
    ASSERT(!ks1.scrollLock, "Default scrollLock is false");

    // Modify
    KeyboardStatus ks2;
    ks2.shiftPressed = true;
    ks2.ctrlPressed = true;
    ks2.capsLock = true;
    ASSERT(ks2.shiftPressed, "Modified shiftPressed");
    ASSERT(ks2.ctrlPressed, "Modified ctrlPressed");
    ASSERT(!ks2.altPressed, "Unmodified altPressed");
    ASSERT(ks2.capsLock, "Modified capsLock");

    // Copy
    KeyboardStatus ks3 = ks2;
    ASSERT(ks3.shiftPressed, "Copy shiftPressed");
    ASSERT(ks3.ctrlPressed, "Copy ctrlPressed");

    Test::PrintSummary();
}

void TestDisplayClass() {
    Test::PrintHeader("Display Class");

    // Test static display mode constants
    Display textMode = Display::TextMode;
    ASSERT_EQ(0x03, static_cast<int>(textMode.Mode()), "TextMode mode is 0x03");

    Display vga320 = Display::VGA_320x200x8;
    ASSERT_EQ(0x13, static_cast<int>(vga320.Mode()), "VGA_320x200x8 mode is 0x13");
    ASSERT_EQ(8, static_cast<int>(vga320.BitsPerPixel()), "VGA_320x200x8 bpp is 8");
    ASSERT_EQ(320, static_cast<int>(vga320.Width()), "VGA_320x200x8 width is 320");
    ASSERT_EQ(200, static_cast<int>(vga320.Height()), "VGA_320x200x8 height is 200");

    Display vga640 = Display::VGA_640x480x4;
    ASSERT_EQ(0x12, static_cast<int>(vga640.Mode()), "VGA_640x480x4 mode is 0x12");
    ASSERT_EQ(4, static_cast<int>(vga640.BitsPerPixel()), "VGA_640x480x4 bpp is 4");
    ASSERT_EQ(640, static_cast<int>(vga640.Width()), "VGA_640x480x4 width is 640");
    ASSERT_EQ(480, static_cast<int>(vga640.Height()), "VGA_640x480x4 height is 480");

    // Copy constructor
    Display copy = vga640;
    ASSERT_EQ(0x12, static_cast<int>(copy.Mode()), "Copy mode");
    ASSERT_EQ(640, static_cast<int>(copy.Width()), "Copy width");

    // Assignment
    Display assigned = textMode;
    assigned = vga320;
    ASSERT_EQ(0x13, static_cast<int>(assigned.Mode()), "Assigned mode");
    ASSERT_EQ(320, static_cast<int>(assigned.Width()), "Assigned width");

    Test::PrintSummary();
}

void TestDisplayEquality() {
    Test::PrintHeader("Display Comparisons");

    Display d1 = Display::VGA_640x480x4;
    Display d2 = Display::VGA_640x480x4;
    Display d3 = Display::VGA_320x200x8;

    // Compare by properties
    ASSERT(d1.Mode() == d2.Mode(), "Same modes equal");
    ASSERT(d1.Width() == d2.Width(), "Same widths equal");
    ASSERT(d1.Height() == d2.Height(), "Same heights equal");
    ASSERT(d1.BitsPerPixel() == d2.BitsPerPixel(), "Same bpp equal");

    ASSERT(d1.Mode() != d3.Mode(), "Different modes not equal");
    ASSERT(d1.Width() != d3.Width(), "Different widths not equal");

    Test::PrintSummary();
}

int main() {
    Console::Clear();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("BCL Devices Test Suite");
    Console::WriteLine("======================");
    Console::ResetColor();
    Console::WriteLine();
    Console::WriteLine("Note: Hardware-dependent tests skipped (require actual hardware)");
    Console::WriteLine();

    TestMouseStatus();
    TestKeyboardStatus();
    TestDisplayClass();
    TestDisplayEquality();

    Console::WriteLine();
    Console::SetForegroundColor(ConsoleColor::White);
    Console::WriteLine("All device tests complete!");
    Console::ResetColor();

    Console::WriteLine();
    Console::Write("Press any key to exit...");
    Console::ReadKey(true);

    return 0;
}
