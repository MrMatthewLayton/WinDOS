#include "test_framework.hpp"

using namespace System;

void TestBoolean() {
    Test::PrintHeader("Boolean");

    // Constructor and conversion
    Boolean b1;
    ASSERT_EQ(false, static_cast<bool>(b1), "Default constructor initializes to false");

    Boolean b2(true);
    ASSERT_EQ(true, static_cast<bool>(b2), "Constructor with true value");

    Boolean b3 = false;
    ASSERT_EQ(false, static_cast<bool>(b3), "Assignment from bool");

    // Operators
    ASSERT_EQ(false, static_cast<bool>(!b2), "Negation operator");
    ASSERT_EQ(false, static_cast<bool>(b2 && b3), "AND operator");
    ASSERT_EQ(true, static_cast<bool>(b2 || b3), "OR operator");

    // Comparison
    ASSERT(b2 == Boolean::True, "Equality comparison");
    ASSERT(b3 != Boolean::True, "Inequality comparison");

    // ToString
    ASSERT(b2.ToString() == "True", "ToString for true");
    ASSERT(b3.ToString() == "False", "ToString for false");

    // Parse
    Boolean parsed;
    ASSERT(Boolean::TryParse("true", parsed) && static_cast<bool>(parsed) == true, "Parse 'true'");
    ASSERT(Boolean::TryParse("False", parsed) && static_cast<bool>(parsed) == false, "Parse 'False'");
    ASSERT(Boolean::TryParse("1", parsed) && static_cast<bool>(parsed) == true, "Parse '1'");
    ASSERT(!Boolean::TryParse("invalid", parsed), "Parse invalid returns false");

    Test::PrintSummary();
}

void TestChar() {
    Test::PrintHeader("Char");

    Char c1;
    ASSERT_EQ('\0', static_cast<char>(c1), "Default constructor initializes to null");

    Char c2('A');
    ASSERT_EQ('A', static_cast<char>(c2), "Constructor with value");

    // Comparison
    ASSERT(Char('A') < Char('B'), "Less than comparison");
    ASSERT(Char('Z') > Char('A'), "Greater than comparison");
    ASSERT(Char('A') == Char('A'), "Equality comparison");

    // Static methods
    ASSERT(Char::IsDigit('5'), "IsDigit returns true for digit");
    ASSERT(!Char::IsDigit('A'), "IsDigit returns false for letter");
    ASSERT(Char::IsLetter('A'), "IsLetter returns true for letter");
    ASSERT(Char::IsLetterOrDigit('5'), "IsLetterOrDigit for digit");
    ASSERT(Char::IsWhiteSpace(' '), "IsWhiteSpace for space");
    ASSERT(Char::IsUpper('A'), "IsUpper for uppercase");
    ASSERT(Char::IsLower('a'), "IsLower for lowercase");
    ASSERT_EQ('a', Char::ToLower('A'), "ToLower");
    ASSERT_EQ('A', Char::ToUpper('a'), "ToUpper");

    // ToString
    ASSERT(c2.ToString() == "A", "ToString");

    Test::PrintSummary();
}

void TestInt32() {
    Test::PrintHeader("Int32");

    Int32 i1;
    ASSERT_EQ(0, static_cast<int>(i1), "Default constructor initializes to 0");

    Int32 i2(42);
    ASSERT_EQ(42, static_cast<int>(i2), "Constructor with value");

    // Arithmetic
    Int32 i3 = i2 + Int32(8);
    ASSERT_EQ(50, static_cast<int>(i3), "Addition operator");

    Int32 i4 = i2 - Int32(2);
    ASSERT_EQ(40, static_cast<int>(i4), "Subtraction operator");

    Int32 i5 = i2 * Int32(2);
    ASSERT_EQ(84, static_cast<int>(i5), "Multiplication operator");

    Int32 i6 = i2 / Int32(2);
    ASSERT_EQ(21, static_cast<int>(i6), "Division operator");

    Int32 i7 = i2 % Int32(5);
    ASSERT_EQ(2, static_cast<int>(i7), "Modulo operator");

    // Unary
    Int32 i8 = -i2;
    ASSERT_EQ(-42, static_cast<int>(i8), "Unary negation");

    // Increment/Decrement
    Int32 i9(10);
    ASSERT_EQ(11, static_cast<int>(++i9), "Pre-increment");
    ASSERT_EQ(11, static_cast<int>(i9++), "Post-increment returns old value");
    ASSERT_EQ(12, static_cast<int>(i9), "Post-increment incremented value");

    // Comparison
    ASSERT(Int32(5) < Int32(10), "Less than");
    ASSERT(Int32(10) > Int32(5), "Greater than");
    ASSERT(Int32(5) == Int32(5), "Equality");
    ASSERT(Int32(5) != Int32(6), "Inequality");
    ASSERT(Int32(5) <= Int32(5), "Less than or equal");
    ASSERT(Int32(5) >= Int32(5), "Greater than or equal");

    // Bitwise
    ASSERT_EQ(0x0F, static_cast<int>(Int32(0xFF) & Int32(0x0F)), "Bitwise AND");
    ASSERT_EQ(0xFF, static_cast<int>(Int32(0xF0) | Int32(0x0F)), "Bitwise OR");
    ASSERT_EQ(0xF0, static_cast<int>(Int32(0xFF) ^ Int32(0x0F)), "Bitwise XOR");
    ASSERT_EQ(4, static_cast<int>(Int32(1) << 2), "Left shift");
    ASSERT_EQ(2, static_cast<int>(Int32(8) >> 2), "Right shift");

    // MinValue/MaxValue
    ASSERT_EQ(-2147483648, Int32::MinValue, "MinValue constant");
    ASSERT_EQ(2147483647, Int32::MaxValue, "MaxValue constant");

    // ToString
    ASSERT(Int32(123).ToString() == "123", "ToString positive");
    ASSERT(Int32(-456).ToString() == "-456", "ToString negative");
    ASSERT(Int32(0).ToString() == "0", "ToString zero");

    // Parse
    Int32 parsed;
    ASSERT(Int32::TryParse("12345", parsed) && static_cast<int>(parsed) == 12345, "Parse positive");
    ASSERT(Int32::TryParse("-67890", parsed) && static_cast<int>(parsed) == -67890, "Parse negative");
    ASSERT(Int32::TryParse("  42  ", parsed) && static_cast<int>(parsed) == 42, "Parse with whitespace");
    ASSERT(!Int32::TryParse("abc", parsed), "Parse invalid returns false");

    // Division by zero
    ASSERT_THROWS(Int32(5) / Int32(0), InvalidOperationException, "Division by zero throws");

    Test::PrintSummary();
}

void TestUInt32() {
    Test::PrintHeader("UInt32");

    UInt32 u1;
    ASSERT_EQ(0u, static_cast<unsigned int>(u1), "Default constructor");

    UInt32 u2(100);
    ASSERT_EQ(100u, static_cast<unsigned int>(u2), "Constructor with value");

    // Arithmetic
    UInt32 u3 = u2 + UInt32(50);
    ASSERT_EQ(150u, static_cast<unsigned int>(u3), "Addition");

    // MinValue/MaxValue
    ASSERT_EQ(0u, UInt32::MinValue, "MinValue constant");
    ASSERT_EQ(4294967295u, UInt32::MaxValue, "MaxValue constant");

    // ToString
    ASSERT(UInt32(12345).ToString() == "12345", "ToString");

    // Parse
    UInt32 parsed;
    ASSERT(UInt32::TryParse("12345", parsed) && static_cast<unsigned int>(parsed) == 12345u, "Parse");
    ASSERT(!UInt32::TryParse("-1", parsed), "Parse negative fails");

    Test::PrintSummary();
}

void TestInt64() {
    Test::PrintHeader("Int64");

    Int64 l1;
    ASSERT_EQ(0LL, static_cast<long long>(l1), "Default constructor");

    Int64 l2(1234567890123LL);
    ASSERT_EQ(1234567890123LL, static_cast<long long>(l2), "Constructor with large value");

    // Arithmetic
    Int64 l3 = l2 + Int64(1);
    ASSERT_EQ(1234567890124LL, static_cast<long long>(l3), "Addition");

    // ToString
    ASSERT(Int64(1234567890123LL).ToString() == "1234567890123", "ToString large number");
    ASSERT(Int64(-9876543210LL).ToString() == "-9876543210", "ToString negative");

    // Parse
    Int64 parsed;
    ASSERT(Int64::TryParse("9223372036854775807", parsed) && static_cast<long long>(parsed) == 9223372036854775807LL, "Parse max value");

    Test::PrintSummary();
}

void TestFloat32() {
    Test::PrintHeader("Float32");

    Float32 f1;
    ASSERT_EQ(0.0f, static_cast<float>(f1), "Default constructor");

    Float32 f2(3.14f);
    ASSERT(static_cast<float>(f2) > 3.13f && static_cast<float>(f2) < 3.15f, "Constructor with value");

    // Arithmetic
    Float32 f3 = f2 + Float32(1.0f);
    ASSERT(static_cast<float>(f3) > 4.13f && static_cast<float>(f3) < 4.15f, "Addition");

    Float32 f4 = Float32(10.0f) / Float32(4.0f);
    ASSERT(static_cast<float>(f4) > 2.49f && static_cast<float>(f4) < 2.51f, "Division");

    // Special values (basic check)
    Float32 inf = Float32(1.0f) / Float32(0.0f);
    ASSERT(Float32::IsInfinity(inf), "IsInfinity");

    // ToString (basic check - just verify it doesn't crash)
    String s = Float32(1.5f).ToString();
    ASSERT(s.Length() > 0, "ToString produces output");

    Test::PrintSummary();
}

void TestFloat64() {
    Test::PrintHeader("Float64");

    Float64 d1;
    ASSERT_EQ(0.0, static_cast<double>(d1), "Default constructor");

    Float64 d2(3.141592653589793);
    ASSERT(static_cast<double>(d2) > 3.14159 && static_cast<double>(d2) < 3.14160, "Constructor with value");

    // Arithmetic
    Float64 d3 = d2 * Float64(2.0);
    ASSERT(static_cast<double>(d3) > 6.28 && static_cast<double>(d3) < 6.29, "Multiplication");

    Test::PrintSummary();
}

void TestSmallIntegers() {
    Test::PrintHeader("Small Integer Types");

    // Int8
    Int8 i8(127);
    ASSERT_EQ(127, static_cast<int>(i8), "Int8 max value");
    ASSERT_EQ(-128, static_cast<int>(Int8::MinValue), "Int8 MinValue");
    ASSERT(Int8(50).ToString() == "50", "Int8 ToString");

    // UInt8
    UInt8 u8(255);
    ASSERT_EQ(255, static_cast<int>(u8), "UInt8 max value");
    ASSERT_EQ(0, static_cast<int>(UInt8::MinValue), "UInt8 MinValue");
    ASSERT(UInt8(200).ToString() == "200", "UInt8 ToString");

    // Int16
    Int16 i16(32767);
    ASSERT_EQ(32767, static_cast<int>(i16), "Int16 max value");
    ASSERT_EQ(-32768, static_cast<int>(Int16::MinValue), "Int16 MinValue");
    ASSERT(Int16(-1000).ToString() == "-1000", "Int16 ToString negative");

    // UInt16
    UInt16 u16(65535);
    ASSERT_EQ(65535, static_cast<int>(u16), "UInt16 max value");
    ASSERT(UInt16(50000).ToString() == "50000", "UInt16 ToString");

    Test::PrintSummary();
}

int main() {
    Console::Clear();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("rtcorlib Types Test Suite");
    Console::WriteLine("====================");
    Console::ResetColor();

    TestBoolean();
    TestChar();
    TestInt32();
    TestUInt32();
    TestInt64();
    TestFloat32();
    TestFloat64();
    TestSmallIntegers();

    Console::WriteLine();
    Console::SetForegroundColor(ConsoleColor::White);
    Console::WriteLine("All type tests complete!");
    Console::ResetColor();

    Console::WriteLine();
    Console::Write("Press any key to exit...");
    Console::ReadKey(true);

    return 0;
}
