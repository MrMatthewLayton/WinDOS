#include "test_framework.hpp"

using namespace System;

void TestConstruction() {
    Test::PrintHeader("Array Construction");

    Array<Int32> a1;
    ASSERT_EQ(0, a1.Length(), "Default constructor creates empty array");
    ASSERT(a1.IsEmpty(), "IsEmpty returns true for empty array");

    Array<Int32> a2(5);
    ASSERT_EQ(5, a2.Length(), "Constructor with size");
    ASSERT_EQ(0, static_cast<int>(a2[0]), "Elements initialized to default");

    Array<Int32> a3 = {1, 2, 3, 4, 5};
    ASSERT_EQ(5, a3.Length(), "Initializer list constructor");
    ASSERT_EQ(1, static_cast<int>(a3[0]), "First element from initializer");
    ASSERT_EQ(5, static_cast<int>(a3[4]), "Last element from initializer");

    Array<Int32> a4(a3);
    ASSERT_EQ(5, a4.Length(), "Copy constructor length");
    ASSERT_EQ(3, static_cast<int>(a4[2]), "Copy constructor content");

    Array<Int32> a5(static_cast<Array<Int32>&&>(Array<Int32>({10, 20, 30})));
    ASSERT_EQ(3, a5.Length(), "Move constructor length");
    ASSERT_EQ(20, static_cast<int>(a5[1]), "Move constructor content");

    ASSERT_THROWS(Array<Int32>(-1), ArgumentOutOfRangeException, "Negative size throws");

    Test::PrintSummary();
}

void TestAssignment() {
    Test::PrintHeader("Array Assignment");

    Array<Int32> a1 = {1, 2, 3};
    Array<Int32> a2 = {4, 5};

    a2 = a1;
    ASSERT_EQ(3, a2.Length(), "Copy assignment length");
    ASSERT_EQ(2, static_cast<int>(a2[1]), "Copy assignment content");

    a2 = {10, 20, 30, 40};
    ASSERT_EQ(4, a2.Length(), "Initializer list assignment length");
    ASSERT_EQ(30, static_cast<int>(a2[2]), "Initializer list assignment content");

    Array<Int32> a3;
    a3 = static_cast<Array<Int32>&&>(Array<Int32>({100, 200}));
    ASSERT_EQ(2, a3.Length(), "Move assignment length");
    ASSERT_EQ(100, static_cast<int>(a3[0]), "Move assignment content");

    Test::PrintSummary();
}

void TestAccess() {
    Test::PrintHeader("Array Access");

    Array<Int32> a = {10, 20, 30, 40, 50};

    ASSERT_EQ(10, static_cast<int>(a[0]), "Index operator first element");
    ASSERT_EQ(50, static_cast<int>(a[4]), "Index operator last element");

    a[2] = 999;
    ASSERT_EQ(999, static_cast<int>(a[2]), "Index operator assignment");

    ASSERT_EQ(20, static_cast<int>(a.GetValue(1)), "GetValue method");

    a.SetValue(3, 888);
    ASSERT_EQ(888, static_cast<int>(a[3]), "SetValue method");

    ASSERT_THROWS(a[-1], IndexOutOfRangeException, "Negative index throws");
    ASSERT_THROWS(a[5], IndexOutOfRangeException, "Out of bounds index throws");
    ASSERT_THROWS(a[100], IndexOutOfRangeException, "Large out of bounds throws");

    // Test const access
    const Array<Int32>& ca = a;
    ASSERT_EQ(10, static_cast<int>(ca[0]), "Const index access");

    Test::PrintSummary();
}

void TestIteration() {
    Test::PrintHeader("Array Iteration");

    Array<Int32> a = {1, 2, 3, 4, 5};

    int sum = 0;
    for (const auto& elem : a) {
        sum += static_cast<int>(elem);
    }
    ASSERT_EQ(15, sum, "Range-based for loop sum");

    // Pointer iteration
    int count = 0;
    for (auto it = a.begin(); it != a.end(); ++it) {
        count++;
    }
    ASSERT_EQ(5, count, "Iterator-based loop count");

    // Modify through iteration
    for (auto& elem : a) {
        elem = elem * Int32(2);
    }
    ASSERT_EQ(2, static_cast<int>(a[0]), "Modified first element");
    ASSERT_EQ(10, static_cast<int>(a[4]), "Modified last element");

    Test::PrintSummary();
}

void TestOperations() {
    Test::PrintHeader("Array Operations");

    Array<Int32> a = {5, 3, 1, 4, 2};

    // Clear
    Array<Int32> a2 = a;
    a2.Clear();
    ASSERT_EQ(0, static_cast<int>(a2[0]), "Clear sets to default");
    ASSERT_EQ(5, a2.Length(), "Clear preserves length");

    // Reverse
    Array<Int32> a3 = {1, 2, 3, 4, 5};
    a3.Reverse();
    ASSERT_EQ(5, static_cast<int>(a3[0]), "Reverse first element");
    ASSERT_EQ(1, static_cast<int>(a3[4]), "Reverse last element");
    ASSERT_EQ(3, static_cast<int>(a3[2]), "Reverse middle element");

    // IndexOf
    Array<Int32> a4 = {10, 20, 30, 20, 40};
    ASSERT_EQ(1, a4.IndexOf(20), "IndexOf finds first occurrence");
    ASSERT_EQ(-1, a4.IndexOf(999), "IndexOf returns -1 for not found");

    // Contains
    ASSERT(a4.Contains(30), "Contains true case");
    ASSERT(!a4.Contains(999), "Contains false case");

    Test::PrintSummary();
}

void TestResize() {
    Test::PrintHeader("Array Resize");

    Array<Int32> a = {1, 2, 3};

    // Grow
    a.Resize(5);
    ASSERT_EQ(5, a.Length(), "Resize grow length");
    ASSERT_EQ(1, static_cast<int>(a[0]), "Resize preserves existing (0)");
    ASSERT_EQ(3, static_cast<int>(a[2]), "Resize preserves existing (2)");
    ASSERT_EQ(0, static_cast<int>(a[3]), "Resize new elements default (3)");
    ASSERT_EQ(0, static_cast<int>(a[4]), "Resize new elements default (4)");

    // Shrink
    a.Resize(2);
    ASSERT_EQ(2, a.Length(), "Resize shrink length");
    ASSERT_EQ(1, static_cast<int>(a[0]), "Resize shrink preserves (0)");
    ASSERT_EQ(2, static_cast<int>(a[1]), "Resize shrink preserves (1)");

    // Resize to zero
    a.Resize(0);
    ASSERT_EQ(0, a.Length(), "Resize to zero");
    ASSERT(a.IsEmpty(), "Resize to zero makes empty");

    // Resize from empty
    a.Resize(3);
    ASSERT_EQ(3, a.Length(), "Resize from empty");

    ASSERT_THROWS(a.Resize(-1), ArgumentOutOfRangeException, "Negative resize throws");

    Test::PrintSummary();
}

void TestCopyTo() {
    Test::PrintHeader("Array CopyTo");

    Array<Int32> src = {1, 2, 3};
    Array<Int32> dst(5);

    src.CopyTo(dst, 1);
    ASSERT_EQ(0, static_cast<int>(dst[0]), "CopyTo destination before");
    ASSERT_EQ(1, static_cast<int>(dst[1]), "CopyTo first copied");
    ASSERT_EQ(2, static_cast<int>(dst[2]), "CopyTo second copied");
    ASSERT_EQ(3, static_cast<int>(dst[3]), "CopyTo third copied");
    ASSERT_EQ(0, static_cast<int>(dst[4]), "CopyTo destination after");

    Array<Int32> small(2);
    ASSERT_THROWS(src.CopyTo(small, 0), ArgumentException, "CopyTo too small throws");
    ASSERT_THROWS(src.CopyTo(dst, -1), ArgumentOutOfRangeException, "CopyTo negative index throws");

    Test::PrintSummary();
}

void TestWithStrings() {
    Test::PrintHeader("Array<String>");

    Array<String> a = {"Hello", "World", "Test"};
    ASSERT_EQ(3, a.Length(), "String array length");
    ASSERT(a[0] == "Hello", "String array element 0");
    ASSERT(a[1] == "World", "String array element 1");

    a[1] = "Universe";
    ASSERT(a[1] == "Universe", "String array modification");

    // Copy
    Array<String> a2 = a;
    ASSERT(a2[0] == "Hello", "String array copy");

    // Reverse
    a.Reverse();
    ASSERT(a[0] == "Test", "String array reverse");

    Test::PrintSummary();
}

void TestFromPointer() {
    Test::PrintHeader("Array FromPointer");

    int rawData[] = {100, 200, 300, 400};
    Array<Int32> a = Array<Int32>::FromPointer(reinterpret_cast<Int32*>(rawData), 4);

    ASSERT_EQ(4, a.Length(), "FromPointer length");
    ASSERT_EQ(100, static_cast<int>(a[0]), "FromPointer element 0");
    ASSERT_EQ(400, static_cast<int>(a[3]), "FromPointer element 3");

    ASSERT_THROWS(Array<Int32>::FromPointer(nullptr, -1), ArgumentOutOfRangeException, "FromPointer negative throws");

    Test::PrintSummary();
}

void TestDataPointer() {
    Test::PrintHeader("Array Data Pointer");

    Array<Int32> a = {1, 2, 3, 4, 5};

    Int32* data = a.Data();
    ASSERT(data != nullptr, "Data returns non-null");
    ASSERT_EQ(1, static_cast<int>(data[0]), "Data access element 0");
    ASSERT_EQ(5, static_cast<int>(data[4]), "Data access element 4");

    // Modify through pointer
    data[2] = 999;
    ASSERT_EQ(999, static_cast<int>(a[2]), "Modification through Data pointer");

    // Const version
    const Array<Int32>& ca = a;
    const Int32* cdata = ca.Data();
    ASSERT_EQ(999, static_cast<int>(cdata[2]), "Const Data access");

    Test::PrintSummary();
}

int main() {
    Console::Clear();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("BCL Array Test Suite");
    Console::WriteLine("====================");
    Console::ResetColor();

    TestConstruction();
    TestAssignment();
    TestAccess();
    TestIteration();
    TestOperations();
    TestResize();
    TestCopyTo();
    TestWithStrings();
    TestFromPointer();
    TestDataPointer();

    Console::WriteLine();
    Console::SetForegroundColor(ConsoleColor::White);
    Console::WriteLine("All array tests complete!");
    Console::ResetColor();

    Console::WriteLine();
    Console::Write("Press any key to exit...");
    Console::ReadKey(true);

    return 0;
}
