// Comprehensive Test Suite for rtcorlib (Retro Technology Core Library)
// Runs ALL tests and produces a comprehensive summary
//
// Build: make test
// Run:   build/bin/test.exe

#include "test_framework.hpp"
#include "../src/rtcorlib.hpp"
#include <cstring>

using namespace System;
using namespace System::Drawing;
using namespace System::IO::Devices;
using namespace System::Windows::Forms;

// Global test counters
static int g_totalPassed = 0;
static int g_totalFailed = 0;
static int g_suitesPassed = 0;
static int g_suitesFailed = 0;

// Capture suite results
void RecordSuiteResults() {
    g_totalPassed += Test::_passCount;
    g_totalFailed += Test::_failCount;
    if (Test::AllPassed()) {
        g_suitesPassed++;
    } else {
        g_suitesFailed++;
    }
}

//=============================================================================
// TYPES TESTS
//=============================================================================

void TestBoolean() {
    Test::PrintHeader("Boolean");
    Boolean b1;
    ASSERT_EQ(false, static_cast<bool>(b1), "Default constructor initializes to false");
    Boolean b2(true);
    ASSERT_EQ(true, static_cast<bool>(b2), "Constructor with true value");
    ASSERT_EQ(false, static_cast<bool>(!b2), "Negation operator");
    ASSERT_EQ(false, static_cast<bool>(b2 && Boolean(false)), "AND operator");
    ASSERT_EQ(true, static_cast<bool>(b2 || Boolean(false)), "OR operator");
    ASSERT(b2 == Boolean::True, "Equality comparison");
    ASSERT(b2.ToString() == "True", "ToString for true");
    Boolean parsed;
    ASSERT(Boolean::TryParse("true", parsed) && static_cast<bool>(parsed) == true, "Parse 'true'");
    ASSERT(!Boolean::TryParse("invalid", parsed), "Parse invalid returns false");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestChar() {
    Test::PrintHeader("Char");
    Char c1;
    ASSERT_EQ('\0', static_cast<char>(c1), "Default constructor");
    Char c2('A');
    ASSERT_EQ('A', static_cast<char>(c2), "Constructor with value");
    ASSERT(Char('A') < Char('B'), "Less than comparison");
    ASSERT(Char::IsDigit('5'), "IsDigit");
    ASSERT(Char::IsLetter('A'), "IsLetter");
    ASSERT(Char::IsWhiteSpace(' '), "IsWhiteSpace");
    ASSERT_EQ('a', Char::ToLower('A'), "ToLower");
    ASSERT_EQ('A', Char::ToUpper('a'), "ToUpper");
    ASSERT(c2.ToString() == "A", "ToString");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestInt32() {
    Test::PrintHeader("Int32");
    Int32 i1;
    ASSERT_EQ(0, static_cast<int>(i1), "Default constructor");
    Int32 i2(42);
    ASSERT_EQ(42, static_cast<int>(i2), "Constructor with value");
    ASSERT_EQ(50, static_cast<int>(i2 + Int32(8)), "Addition");
    ASSERT_EQ(40, static_cast<int>(i2 - Int32(2)), "Subtraction");
    ASSERT_EQ(84, static_cast<int>(i2 * Int32(2)), "Multiplication");
    ASSERT_EQ(21, static_cast<int>(i2 / Int32(2)), "Division");
    ASSERT_EQ(2, static_cast<int>(i2 % Int32(5)), "Modulo");
    ASSERT_EQ(-42, static_cast<int>(-i2), "Unary negation");
    ASSERT(Int32(5) < Int32(10), "Less than");
    ASSERT(Int32(5) == Int32(5), "Equality");
    ASSERT(Int32(123).ToString() == "123", "ToString");
    Int32 parsed;
    ASSERT(Int32::TryParse("12345", parsed) && static_cast<int>(parsed) == 12345, "Parse");
    ASSERT_THROWS(Int32(5) / Int32(0), InvalidOperationException, "Division by zero throws");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestFloat64() {
    Test::PrintHeader("Float64");
    Float64 d1;
    ASSERT_EQ(0.0, static_cast<double>(d1), "Default constructor");
    Float64 d2(3.14);
    ASSERT(static_cast<double>(d2) > 3.13 && static_cast<double>(d2) < 3.15, "Constructor with value");
    Float64 d3 = d2 * Float64(2.0);
    // Use >= for lower bound since 3.14 * 2.0 = 6.28 exactly
    ASSERT(static_cast<double>(d3) >= 6.28 && static_cast<double>(d3) < 6.29, "Multiplication");
    Float64 inf = Float64(1.0) / Float64(0.0);
    ASSERT(Float64::IsInfinity(inf), "IsInfinity");
    Test::PrintSummary();
    RecordSuiteResults();
}

//=============================================================================
// STRING TESTS
//=============================================================================

void TestStringBasic() {
    Test::PrintHeader("String Basic");
    String s1;
    ASSERT(s1.Length() == 0, "Default constructor");
    ASSERT(s1.IsEmpty(), "IsEmpty");
    String s2("Hello");
    ASSERT_EQ(5, s2.Length(), "Constructor from C string");
    ASSERT(s2 == "Hello", "Content matches");
    String s3('*', 5);
    ASSERT(s3 == "*****", "Fill constructor");
    String s4(s2);
    ASSERT(s4 == s2, "Copy constructor");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestStringOperations() {
    Test::PrintHeader("String Operations");
    String s("Hello World");
    ASSERT(s.Substring(0, 5) == "Hello", "Substring");
    ASSERT_EQ(6, s.IndexOf("World"), "IndexOf");
    ASSERT(s.Contains("World"), "Contains");
    ASSERT(s.StartsWith("Hello"), "StartsWith");
    ASSERT(s.EndsWith("World"), "EndsWith");
    ASSERT(s.ToUpper() == "HELLO WORLD", "ToUpper");
    ASSERT(s.ToLower() == "hello world", "ToLower");
    String padded("  test  ");
    ASSERT(padded.Trim() == "test", "Trim");
    ASSERT(s.Replace("World", "DOS") == "Hello DOS", "Replace");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestStringConcat() {
    Test::PrintHeader("String Concatenation");
    String s1("Hello");
    String s2(" World");
    ASSERT(s1 + s2 == "Hello World", "Concatenation operator");
    ASSERT(s1 + "!" == "Hello!", "Concatenation with C string");
    ASSERT("Say " + s1 == "Say Hello", "C string + String");
    String s3 = s1;
    s3 += s2;
    ASSERT(s3 == "Hello World", "Compound assignment");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestStringSplit() {
    Test::PrintHeader("String Split");
    String s("one,two,three");
    Array<String> parts = s.Split(',');
    ASSERT_EQ(3, parts.Length(), "Split count");
    ASSERT(parts[0] == "one", "Split part 0");
    ASSERT(parts[1] == "two", "Split part 1");
    ASSERT(parts[2] == "three", "Split part 2");
    Test::PrintSummary();
    RecordSuiteResults();
}

//=============================================================================
// ARRAY TESTS
//=============================================================================

void TestArrayBasic() {
    Test::PrintHeader("Array Basic");
    Array<Int32> a1;
    ASSERT_EQ(0, a1.Length(), "Default constructor");
    ASSERT(a1.IsEmpty(), "IsEmpty");
    Array<Int32> a2(5);
    ASSERT_EQ(5, a2.Length(), "Constructor with size");
    Array<Int32> a3 = {1, 2, 3, 4, 5};
    ASSERT_EQ(5, a3.Length(), "Initializer list");
    ASSERT_EQ(1, static_cast<int>(a3[0]), "First element");
    ASSERT_EQ(5, static_cast<int>(a3[4]), "Last element");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestArrayOperations() {
    Test::PrintHeader("Array Operations");
    Array<Int32> a = {5, 3, 1, 4, 2};
    ASSERT_EQ(5, a.Length(), "Length");
    a[2] = 999;
    ASSERT_EQ(999, static_cast<int>(a[2]), "Index assignment");
    Array<Int32> a2 = {1, 2, 3, 4, 5};
    a2.Reverse();
    ASSERT_EQ(5, static_cast<int>(a2[0]), "Reverse first");
    ASSERT_EQ(1, static_cast<int>(a2[4]), "Reverse last");
    Array<Int32> a3 = {10, 20, 30, 20, 40};
    ASSERT_EQ(1, a3.IndexOf(20), "IndexOf");
    ASSERT(a3.Contains(30), "Contains");
    ASSERT(!a3.Contains(999), "Contains false");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestArrayBounds() {
    Test::PrintHeader("Array Bounds Checking");
    Array<Int32> a = {1, 2, 3, 4, 5};
    ASSERT_THROWS(a[-1], IndexOutOfRangeException, "Negative index throws");
    ASSERT_THROWS(a[5], IndexOutOfRangeException, "Out of bounds throws");
    ASSERT_THROWS(a[100], IndexOutOfRangeException, "Large index throws");
    ASSERT_THROWS(Array<Int32>(-1), ArgumentOutOfRangeException, "Negative size throws");
    Test::PrintSummary();
    RecordSuiteResults();
}

//=============================================================================
// EXCEPTION TESTS
//=============================================================================

void TestExceptionHierarchy() {
    Test::PrintHeader("Exception Hierarchy");
    Exception e1("Test message");
    ASSERT(std::strcmp(e1.Message(), "Test message") == 0, "Exception message");
    ArgumentException ae("Invalid", "param");
    ASSERT(std::strcmp(ae.ParamName(), "param") == 0, "ArgumentException ParamName");
    ArgumentNullException ane("value");
    ASSERT(std::strstr(ane.Message(), "null") != nullptr, "ArgumentNullException message");
    IndexOutOfRangeException iore;
    ASSERT(std::strstr(iore.Message(), "bounds") != nullptr || std::strstr(iore.Message(), "Index") != nullptr, "IndexOutOfRangeException message");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestExceptionThrowCatch() {
    Test::PrintHeader("Exception Throw/Catch");
    bool caught = false;
    try {
        throw Exception("Test");
    } catch (const Exception&) {
        caught = true;
    }
    ASSERT(caught, "Exception caught");
    caught = false;
    try {
        throw IndexOutOfRangeException();
    } catch (const Exception&) {
        caught = true;
    }
    ASSERT(caught, "Derived caught as base");
    caught = false;
    try {
        Array<Int32> arr(5);
        int x = arr[10];
        (void)x;
    } catch (const IndexOutOfRangeException&) {
        caught = true;
    }
    ASSERT(caught, "Array throws IndexOutOfRangeException");
    Test::PrintSummary();
    RecordSuiteResults();
}

//=============================================================================
// CONSOLE TESTS (minimal, visual verification needed)
//=============================================================================

void TestConsoleBasic() {
    Test::PrintHeader("Console Basic");
    Console::Write("Test output... ");
    Console::WriteLine("OK");
    Test::Pass("Write and WriteLine work");
    Console::SetForegroundColor(ConsoleColor::Green);
    Console::Write("Green ");
    Console::SetForegroundColor(ConsoleColor::Red);
    Console::Write("Red ");
    Console::ResetColor();
    Console::WriteLine();
    Test::Pass("Colors work");
    ASSERT(static_cast<int>(Console::WindowWidth()) > 0, "WindowWidth positive");
    ASSERT(static_cast<int>(Console::WindowHeight()) > 0, "WindowHeight positive");
    Test::PrintSummary();
    RecordSuiteResults();
}

//=============================================================================
// DRAWING TESTS
//=============================================================================

void TestColor() {
    Test::PrintHeader("Color");
    Color c1;
    ASSERT_EQ(0xFF000000u, static_cast<unsigned int>(c1.ToArgb()), "Default is opaque black");
    ASSERT_EQ(0xFFFFFFFFu, static_cast<unsigned int>(Color::White.ToArgb()), "White is 0xFFFFFFFF");
    ASSERT_EQ(0x00000000u, static_cast<unsigned int>(Color::Transparent.ToArgb()), "Transparent is 0x00000000");
    ASSERT(Color::Red == Color::Red, "Equality");
    ASSERT(Color::Red != Color::Blue, "Inequality");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestPoint() {
    Test::PrintHeader("Point");
    Point p1;
    ASSERT_EQ(0, p1.x, "Default x");
    ASSERT_EQ(0, p1.y, "Default y");
    Point p2(10, 20);
    ASSERT_EQ(10, p2.x, "Constructor x");
    Point p3 = p2.Offset(5, -5);
    ASSERT_EQ(15, p3.x, "Offset x");
    ASSERT_EQ(15, p3.y, "Offset y");
    ASSERT(Point::Empty == Point(0, 0), "Empty");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestSize() {
    Test::PrintHeader("Size");
    Size s1;
    ASSERT(s1.IsEmpty(), "Default is empty");
    Size s2(100, 200);
    ASSERT(!s2.IsEmpty(), "Non-zero not empty");
    ASSERT_EQ(100, s2.width, "Width");
    ASSERT_EQ(200, s2.height, "Height");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestRectangle() {
    Test::PrintHeader("Rectangle");
    Rectangle r(10, 20, 100, 50);
    ASSERT_EQ(10, r.Left(), "Left");
    ASSERT_EQ(20, r.Top(), "Top");
    ASSERT_EQ(110, r.Right(), "Right");
    ASSERT_EQ(70, r.Bottom(), "Bottom");
    ASSERT(r.Contains(50, 40), "Contains inside");
    ASSERT(!r.Contains(5, 40), "Not contains outside");
    Rectangle r2 = r.Offset(5, 10);
    ASSERT_EQ(15, r2.x, "Offset x");
    Rectangle r3 = r.Inflate(10, 10);
    ASSERT_EQ(0, r3.x, "Inflate x");
    ASSERT_EQ(120, r3.width, "Inflate width");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestImage() {
    Test::PrintHeader("Image");
    Image img1;
    ASSERT_EQ(0, img1.Width(), "Default width");
    Image img2(10, 20, Color::Blue);
    ASSERT_EQ(10, img2.Width(), "Constructor width");
    ASSERT_EQ(200, img2.Length(), "Length");
    ASSERT(Color::Blue == img2.GetPixel(0, 0), "Fill color");
    img2.SetPixel(5, 5, Color::Red);
    ASSERT(Color::Red == img2.GetPixel(5, 5), "SetPixel");
    img2.Clear(Color::Green);
    ASSERT(Color::Green == img2.GetPixel(5, 5), "Clear");
    Image region = img2.GetRegion(2, 2, 5, 5);
    ASSERT_EQ(5, region.Width(), "Region width");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestGraphicsBuffer() {
    Test::PrintHeader("GraphicsBuffer");
    GraphicsBuffer::CreateFrameBuffer(100, 100, 0x12);
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    ASSERT(fb != nullptr, "CreateFrameBuffer creates buffer");
    ASSERT_EQ(100, fb->Bounds().width, "Buffer width");
    ASSERT_EQ(100, fb->GetImage().Width(), "Image width");
    GraphicsBuffer::DestroyFrameBuffer();
    ASSERT(GraphicsBuffer::GetFrameBuffer() == nullptr, "DestroyFrameBuffer clears");
    Test::PrintSummary();
    RecordSuiteResults();
}

//=============================================================================
// DEVICES TESTS
//=============================================================================

void TestMouseStatus() {
    Test::PrintHeader("MouseStatus");
    MouseStatus ms1;
    ASSERT_EQ(0, ms1.x, "Default x");
    ASSERT(!ms1.leftButton, "Default left");
    MouseStatus ms2(100, 200, true, false, true);
    ASSERT_EQ(100, ms2.x, "Constructor x");
    ASSERT(ms2.leftButton, "Constructor left");
    ASSERT(ms2.middleButton, "Constructor middle");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestKeyboardStatus() {
    Test::PrintHeader("KeyboardStatus");
    KeyboardStatus ks;
    ASSERT(!ks.shiftPressed, "Default shift");
    ASSERT(!ks.ctrlPressed, "Default ctrl");
    ASSERT(!ks.altPressed, "Default alt");
    ks.shiftPressed = true;
    ASSERT(ks.shiftPressed, "Modified shift");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestDisplayClass() {
    Test::PrintHeader("Display");
    Display vga640 = Display::VGA_640x480x4;
    ASSERT_EQ(0x12, static_cast<int>(vga640.Mode()), "VGA mode");
    ASSERT_EQ(640, static_cast<int>(vga640.Width()), "VGA width");
    ASSERT_EQ(480, static_cast<int>(vga640.Height()), "VGA height");
    Display vga320 = Display::VGA_320x200x8;
    ASSERT_EQ(0x13, static_cast<int>(vga320.Mode()), "320x200 mode");
    Test::PrintSummary();
    RecordSuiteResults();
}

//=============================================================================
// FORMS TESTS
//=============================================================================

void TestEventArgs() {
    Test::PrintHeader("Event Args");
    PaintEventArgs pe;
    ASSERT(pe.graphics == nullptr, "PaintEventArgs default graphics");
    MouseEventArgs me(100, 200, true, false);
    ASSERT_EQ(100, me.x, "MouseEventArgs x");
    ASSERT(me.leftButton, "MouseEventArgs left");
    KeyboardEventArgs ke('A', false, true, false);
    ASSERT_EQ('A', ke.key, "KeyboardEventArgs key");
    ASSERT(ke.ctrl, "KeyboardEventArgs ctrl");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestControl() {
    Test::PrintHeader("Control");
    Control c1;
    ASSERT(c1.Parent() == nullptr, "Default parent nullptr");
    ASSERT_EQ(0, c1.ChildCount(), "Default no children");
    Control c2(nullptr, Rectangle(10, 20, 100, 50));
    ASSERT_EQ(10, c2.Bounds().x, "Bounds x");
    ASSERT_EQ(100, c2.Bounds().width, "Bounds width");
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestControlHierarchy() {
    Test::PrintHeader("Control Hierarchy");
    Control parent(nullptr, Rectangle(0, 0, 200, 200));
    Control* child1 = new Control(&parent, Rectangle(10, 10, 50, 50));
    Control* child2 = new Control(&parent, Rectangle(70, 10, 50, 50));
    (void)child2;  // Used for testing ChildCount - deleted by parent destructor
    ASSERT_EQ(2, parent.ChildCount(), "Parent has 2 children");
    ASSERT(parent.GetChild(0) == child1, "First child");
    ASSERT(child1->Parent() == &parent, "Child parent");
    parent.RemoveChild(child1);
    ASSERT_EQ(1, parent.ChildCount(), "After remove");
    ASSERT(child1->Parent() == nullptr, "Removed child parent");
    // Only delete child1 since it was removed from parent.
    // child2 is still owned by parent and will be deleted by parent destructor.
    delete child1;
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestControlBounds() {
    Test::PrintHeader("Control Bounds");
    Control parent(nullptr, Rectangle(100, 100, 300, 200));
    Control* child = new Control(&parent, Rectangle(10, 10, 50, 50));
    Rectangle screen = child->ScreenBounds();
    ASSERT_EQ(110, screen.x, "Screen bounds x");
    ASSERT_EQ(110, screen.y, "Screen bounds y");
    ASSERT(parent.HitTest(150, 150), "HitTest inside");
    ASSERT(!parent.HitTest(50, 50), "HitTest outside");
    // child is owned by parent - will be deleted by parent destructor
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestWindowControl() {
    Test::PrintHeader("Window");
    Control parent(nullptr, Rectangle(0, 0, 640, 480));
    Window* window = new Window(&parent, Rectangle(50, 50, 200, 150));
    ASSERT_EQ(50, window->Bounds().x, "Window bounds x");
    ASSERT(window->ClientBounds().width < 200, "Client width less than total");
    ASSERT(!window->IsFocused(), "Not focused initially");
    window->SetFocused(true);
    ASSERT(window->IsFocused(), "Focused after set");
    // window is owned by parent - will be deleted by parent destructor
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestButtonControl() {
    Test::PrintHeader("Button");
    Control parent(nullptr, Rectangle(0, 0, 200, 200));
    Button* button = new Button(&parent, Rectangle(10, 10, 80, 30));
    ASSERT(!button->IsPressed(), "Not pressed initially");
    ASSERT_EQ(80, button->Bounds().width, "Button width");
    // button is owned by parent - will be deleted by parent destructor
    Test::PrintSummary();
    RecordSuiteResults();
}

void TestPictureControl() {
    Test::PrintHeader("Picture");
    Control parent(nullptr, Rectangle(0, 0, 200, 200));
    Picture* pic = new Picture(&parent, Rectangle(10, 10, 50, 50));
    ASSERT_EQ(0, pic->GetImage().Width(), "Empty image initially");
    Image img(30, 30, Color::Red);
    pic->SetImage(img);
    ASSERT_EQ(30, pic->GetImage().Width(), "SetImage width");
    // pic is owned by parent - will be deleted by parent destructor
    Test::PrintSummary();
    RecordSuiteResults();
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    Console::Clear();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("================================================================================");
    Console::WriteLine("                rtcorlib COMPREHENSIVE TEST SUITE");
    Console::WriteLine("================================================================================");
    Console::ResetColor();
    Console::WriteLine();

    // Types tests
    Console::SetForegroundColor(ConsoleColor::Yellow);
    Console::WriteLine("--- TYPES TESTS ---");
    Console::ResetColor();
    TestBoolean();
    TestChar();
    TestInt32();
    TestFloat64();

    // String tests
    Console::SetForegroundColor(ConsoleColor::Yellow);
    Console::WriteLine("--- STRING TESTS ---");
    Console::ResetColor();
    TestStringBasic();
    TestStringOperations();
    TestStringConcat();
    TestStringSplit();

    // Array tests
    Console::SetForegroundColor(ConsoleColor::Yellow);
    Console::WriteLine("--- ARRAY TESTS ---");
    Console::ResetColor();
    TestArrayBasic();
    TestArrayOperations();
    TestArrayBounds();

    // Exception tests
    Console::SetForegroundColor(ConsoleColor::Yellow);
    Console::WriteLine("--- EXCEPTION TESTS ---");
    Console::ResetColor();
    TestExceptionHierarchy();
    TestExceptionThrowCatch();

    // Console tests
    Console::SetForegroundColor(ConsoleColor::Yellow);
    Console::WriteLine("--- CONSOLE TESTS ---");
    Console::ResetColor();
    TestConsoleBasic();

    // Drawing tests
    Console::SetForegroundColor(ConsoleColor::Yellow);
    Console::WriteLine("--- DRAWING TESTS ---");
    Console::ResetColor();
    TestColor();
    TestPoint();
    TestSize();
    TestRectangle();
    TestImage();
    TestGraphicsBuffer();

    // Devices tests
    Console::SetForegroundColor(ConsoleColor::Yellow);
    Console::WriteLine("--- DEVICES TESTS ---");
    Console::ResetColor();
    TestMouseStatus();
    TestKeyboardStatus();
    TestDisplayClass();

    // Forms tests
    Console::SetForegroundColor(ConsoleColor::Yellow);
    Console::WriteLine("--- FORMS TESTS ---");
    Console::ResetColor();
    TestEventArgs();
    TestControl();
    TestControlHierarchy();
    TestControlBounds();
    TestWindowControl();
    TestButtonControl();
    TestPictureControl();

    // Final summary
    Console::WriteLine();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("================================================================================");
    Console::WriteLine("                           FINAL SUMMARY");
    Console::WriteLine("================================================================================");
    Console::ResetColor();
    Console::WriteLine();

    Console::Write("Test Suites: ");
    if (g_suitesFailed == 0) {
        Console::SetForegroundColor(ConsoleColor::Green);
    } else {
        Console::SetForegroundColor(ConsoleColor::Red);
    }
    Console::Write(g_suitesPassed);
    Console::Write("/");
    Console::Write(g_suitesPassed + g_suitesFailed);
    Console::WriteLine(" passed");
    Console::ResetColor();

    Console::Write("Total Tests: ");
    if (g_totalFailed == 0) {
        Console::SetForegroundColor(ConsoleColor::Green);
    } else {
        Console::SetForegroundColor(ConsoleColor::Red);
    }
    Console::Write(g_totalPassed);
    Console::Write("/");
    Console::Write(g_totalPassed + g_totalFailed);
    Console::WriteLine(" passed");
    Console::ResetColor();

    Console::WriteLine();
    if (g_totalFailed == 0) {
        Console::SetForegroundColor(ConsoleColor::Green);
        Console::WriteLine("*** ALL TESTS PASSED ***");
    } else {
        Console::SetForegroundColor(ConsoleColor::Red);
        Console::Write("*** ");
        Console::Write(g_totalFailed);
        Console::WriteLine(" TESTS FAILED ***");
    }
    Console::ResetColor();

    Console::WriteLine();
    // Skip ReadKey for headless CI testing
    // Console::Write("Press any key to exit...");
    // Console::ReadKey(true);

    return g_totalFailed > 0 ? 1 : 0;
}
