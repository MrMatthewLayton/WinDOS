#include "test_framework.hpp"
#include <cstring>

using namespace System;

void TestConstruction() {
    Test::PrintHeader("String Construction");

    String s1;
    ASSERT(s1.Length() == 0, "Default constructor creates empty string");
    ASSERT(s1.IsEmpty(), "IsEmpty returns true for empty string");

    String s2("Hello");
    ASSERT_EQ(5, s2.Length(), "Constructor from C string");
    ASSERT(s2 == "Hello", "Content matches");

    String s3("Hello World", 5);
    ASSERT(s3 == "Hello", "Constructor with length parameter");

    String s4('*', 5);
    ASSERT(s4 == "*****", "Fill constructor");

    String s5(s2);
    ASSERT(s5 == s2, "Copy constructor");

    String s6(static_cast<String&&>(String("Moved")));
    ASSERT(s6 == "Moved", "Move constructor");

    Test::PrintSummary();
}

void TestAssignment() {
    Test::PrintHeader("String Assignment");

    String s1 = "First";
    String s2 = "Second";

    s1 = s2;
    ASSERT(s1 == "Second", "Copy assignment");

    s1 = "Third";
    ASSERT(s1 == "Third", "Assignment from C string");

    s1 = static_cast<String&&>(String("Fourth"));
    ASSERT(s1 == "Fourth", "Move assignment");

    Test::PrintSummary();
}

void TestAccess() {
    Test::PrintHeader("String Access");

    String s("Hello");

    ASSERT_EQ('H', s[0], "Index access first char");
    ASSERT_EQ('o', s[4], "Index access last char");

    ASSERT_THROWS(s[-1], IndexOutOfRangeException, "Negative index throws");
    ASSERT_THROWS(s[5], IndexOutOfRangeException, "Out of bounds index throws");

    ASSERT(std::strcmp(s.CStr(), "Hello") == 0, "CStr returns correct pointer");

    Test::PrintSummary();
}

void TestSubstring() {
    Test::PrintHeader("String Substring");

    String s("Hello World");

    ASSERT(s.Substring(0, 5) == "Hello", "Substring from start");
    ASSERT(s.Substring(6) == "World", "Substring to end");
    ASSERT(s.Substring(6, 5) == "World", "Substring with length");
    ASSERT(s.Substring(0, 0) == "", "Empty substring");

    ASSERT_THROWS(s.Substring(-1), ArgumentOutOfRangeException, "Negative startIndex throws");
    ASSERT_THROWS(s.Substring(0, 20), ArgumentOutOfRangeException, "Length too long throws");

    Test::PrintSummary();
}

void TestSearch() {
    Test::PrintHeader("String Search");

    String s("Hello World Hello");

    // IndexOf char
    ASSERT_EQ(0, s.IndexOf('H'), "IndexOf first char");
    ASSERT_EQ(2, s.IndexOf('l'), "IndexOf finds first occurrence");
    ASSERT_EQ(-1, s.IndexOf('z'), "IndexOf returns -1 for not found");
    ASSERT_EQ(4, s.IndexOf('o', 3), "IndexOf with startIndex");

    // IndexOf string
    ASSERT_EQ(6, s.IndexOf("World"), "IndexOf string");
    ASSERT_EQ(12, s.IndexOf("Hello", 1), "IndexOf string with startIndex");
    ASSERT_EQ(-1, s.IndexOf("xyz"), "IndexOf string not found");

    // LastIndexOf
    ASSERT_EQ(12, s.LastIndexOf('H'), "LastIndexOf char");
    ASSERT_EQ(12, s.LastIndexOf("Hello"), "LastIndexOf string");

    // Contains
    ASSERT(s.Contains("World"), "Contains finds substring");
    ASSERT(!s.Contains("xyz"), "Contains returns false for missing");

    // StartsWith/EndsWith
    ASSERT(s.StartsWith("Hello"), "StartsWith true case");
    ASSERT(!s.StartsWith("World"), "StartsWith false case");
    ASSERT(s.EndsWith("Hello"), "EndsWith true case");
    ASSERT(!s.EndsWith("World"), "EndsWith false case");

    Test::PrintSummary();
}

void TestTransform() {
    Test::PrintHeader("String Transform");

    String s("  Hello World  ");

    ASSERT(s.Trim() == "Hello World", "Trim removes whitespace");
    ASSERT(s.TrimStart() == "Hello World  ", "TrimStart");
    ASSERT(s.TrimEnd() == "  Hello World", "TrimEnd");

    String s2("Hello World");
    ASSERT(s2.ToUpper() == "HELLO WORLD", "ToUpper");
    ASSERT(s2.ToLower() == "hello world", "ToLower");

    ASSERT(s2.Replace('o', '0') == "Hell0 W0rld", "Replace char");
    ASSERT(s2.Replace("World", "Universe") == "Hello Universe", "Replace string");

    ASSERT(s2.Insert(5, " Beautiful") == "Hello Beautiful World", "Insert");
    ASSERT(s2.Remove(5) == "Hello", "Remove to end");
    ASSERT(s2.Remove(5, 1) == "HelloWorld", "Remove count");

    String s3("Hi");
    ASSERT(s3.PadLeft(5) == "   Hi", "PadLeft with spaces");
    ASSERT(s3.PadLeft(5, '*') == "***Hi", "PadLeft with char");
    ASSERT(s3.PadRight(5) == "Hi   ", "PadRight with spaces");

    Test::PrintSummary();
}

void TestSplit() {
    Test::PrintHeader("String Split");

    String s("one,two,three");
    Array<String> parts = s.Split(',');

    ASSERT_EQ(3, parts.Length(), "Split creates correct number of parts");
    ASSERT(parts[0] == "one", "Split part 0");
    ASSERT(parts[1] == "two", "Split part 1");
    ASSERT(parts[2] == "three", "Split part 2");

    String s2("a-b;c-d");
    Array<String> parts2 = s2.Split("-;");
    ASSERT_EQ(4, parts2.Length(), "Split with multiple delimiters");
    ASSERT(parts2[0] == "a", "Multi-delim part 0");
    ASSERT(parts2[3] == "d", "Multi-delim part 3");

    String s3("no delimiters");
    Array<String> parts3 = s3.Split(',');
    ASSERT_EQ(1, parts3.Length(), "Split with no matches returns original");
    ASSERT(parts3[0] == "no delimiters", "Split no match content");

    Test::PrintSummary();
}

void TestComparison() {
    Test::PrintHeader("String Comparison");

    String s1("apple");
    String s2("banana");
    String s3("apple");
    String s4("APPLE");

    ASSERT(s1 == s3, "Equality operator");
    ASSERT(s1 != s2, "Inequality operator");
    ASSERT(s1 < s2, "Less than operator");
    ASSERT(s2 > s1, "Greater than operator");
    ASSERT(s1 <= s3, "Less than or equal (equal)");
    ASSERT(s1 <= s2, "Less than or equal (less)");

    ASSERT(s1 == "apple", "Equality with C string");
    ASSERT(s1 != "orange", "Inequality with C string");

    ASSERT(s1.Equals(s3), "Equals method");
    ASSERT(s1.EqualsIgnoreCase(s4), "EqualsIgnoreCase");

    ASSERT_EQ(0, s1.CompareTo(s3), "CompareTo equal");
    ASSERT(s1.CompareTo(s2) < 0, "CompareTo less");
    ASSERT(s2.CompareTo(s1) > 0, "CompareTo greater");

    ASSERT(String::Compare(s1, s3) == 0, "Static Compare");
    ASSERT(String::CompareIgnoreCase(s1, s4) == 0, "Static CompareIgnoreCase");

    Test::PrintSummary();
}

void TestConcatenation() {
    Test::PrintHeader("String Concatenation");

    String s1("Hello");
    String s2(" World");

    String s3 = s1 + s2;
    ASSERT(s3 == "Hello World", "Concatenation operator");

    String s4 = s1 + "!";
    ASSERT(s4 == "Hello!", "Concatenation with C string");

    String s5 = s1 + '!';
    ASSERT(s5 == "Hello!", "Concatenation with char");

    String s6 = "Say " + s1;
    ASSERT(s6 == "Say Hello", "C string + String");

    String s7 = s1;
    s7 += s2;
    ASSERT(s7 == "Hello World", "Compound assignment +=");

    ASSERT(String::Concat(s1, s2) == "Hello World", "Static Concat 2 args");
    ASSERT(String::Concat(s1, " ", s2) == "Hello  World", "Static Concat 3 args");

    Test::PrintSummary();
}

void TestStaticMethods() {
    Test::PrintHeader("String Static Methods");

    ASSERT(String::IsNullOrEmpty(String()), "IsNullOrEmpty for empty");
    ASSERT(String::IsNullOrEmpty(String("")), "IsNullOrEmpty for empty C string");
    ASSERT(!String::IsNullOrEmpty(String("x")), "IsNullOrEmpty for non-empty");

    ASSERT(String::IsNullOrWhiteSpace(String()), "IsNullOrWhiteSpace for empty");
    ASSERT(String::IsNullOrWhiteSpace(String("   ")), "IsNullOrWhiteSpace for spaces");
    ASSERT(String::IsNullOrWhiteSpace(String("\t\n")), "IsNullOrWhiteSpace for whitespace");
    ASSERT(!String::IsNullOrWhiteSpace(String(" x ")), "IsNullOrWhiteSpace for non-empty");

    ASSERT(String::Empty.Length() == 0, "String::Empty is empty");

    Test::PrintSummary();
}

void TestHashCode() {
    Test::PrintHeader("String HashCode");

    String s1("Hello");
    String s2("Hello");
    String s3("World");

    ASSERT(s1.GetHashCode() == s2.GetHashCode(), "Equal strings have equal hash");
    ASSERT(s1.GetHashCode() != s3.GetHashCode(), "Different strings likely have different hash");
    ASSERT(String().GetHashCode() == 0, "Empty string hash is 0");

    Test::PrintSummary();
}

void TestImmutability() {
    Test::PrintHeader("String Immutability");

    String original("Hello");
    String upper = original.ToUpper();

    ASSERT(original == "Hello", "Original unchanged after ToUpper");
    ASSERT(upper == "HELLO", "ToUpper returns new string");

    String trimmed = String("  Hi  ").Trim();
    ASSERT(trimmed == "Hi", "Trim returns new string");

    String replaced = original.Replace('l', 'L');
    ASSERT(original == "Hello", "Original unchanged after Replace");
    ASSERT(replaced == "HeLLo", "Replace returns new string");

    Test::PrintSummary();
}

int main() {
    Console::Clear();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("BCL String Test Suite");
    Console::WriteLine("=====================");
    Console::ResetColor();

    TestConstruction();
    TestAssignment();
    TestAccess();
    TestSubstring();
    TestSearch();
    TestTransform();
    TestSplit();
    TestComparison();
    TestConcatenation();
    TestStaticMethods();
    TestHashCode();
    TestImmutability();

    Console::WriteLine();
    Console::SetForegroundColor(ConsoleColor::White);
    Console::WriteLine("All string tests complete!");
    Console::ResetColor();

    Console::WriteLine();
    Console::Write("Press any key to exit...");
    Console::ReadKey(true);

    return 0;
}
