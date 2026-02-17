# WinDOS - Base Class Library for DOS

## Introduction

WinDOS is a .NET-inspired Base Class Library (BCL) that brings modern programming paradigms to DOS. It provides a comprehensive API for building graphical applications with a Windows 95-style user interface, all running in DOS protected mode.

### Features

- **Modern C++17** - Smart memory management, templates, move semantics
- **.NET-style APIs** - Familiar type wrappers, String, Array, exceptions
- **VGA Graphics** - 640x480 16-color and 320x200 256-color modes
- **VBE High-Resolution** - 800x600+ with linear framebuffer and true color
- **Windows Forms** - Desktop, windows, taskbar, buttons, icons, and more
- **Flexbox Layout System** - CSS-inspired automatic layout with flex containers
- **Hardware Abstraction** - Clean APIs for mouse, keyboard, and display
- **File I/O** - File reading with exception-based error handling
- **Icon Support** - Load icons from .ico files and PE executables (.dll, .exe)

### Requirements

- DJGPP cross-compiler (`i586-pc-msdosdjgpp-g++`)
- CWSDPMI.EXE (DOS Protected Mode Interface)
- DOS or DOS emulator (DOSBox, FreeDOS, etc.)

### Quick Start

```cpp
#include "bcl/BCL.hpp"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

int main() {
    // Create a Windows 95-style desktop
    Desktop desktop(Color::Cyan);

    // Add a window
    Window* window = new Window(&desktop, Rectangle(100, 100, 300, 200));

    // Run the event loop (ESC to exit)
    desktop.Run();

    return 0;
}
```

---

# API Reference

## Table of Contents

1. [System Namespace](#system-namespace)
   - [Primitive Types](#primitive-types)
   - [Math Class](#math-class)
   - [String Class](#string-class)
   - [StringBuilder Class](#stringbuilder-class)
   - [MemoryPool Class](#memorypool-class)
   - [StringIntern Class](#stringintern-class)
   - [Array Class](#array-class)
   - [Exception Classes](#exception-classes)
   - [Console Class](#console-class)
2. [System.IO Namespace](#systemio-namespace)
   - [File Class](#file-class)
3. [System.Drawing Namespace](#systemdrawing-namespace)
   - [Color Structure](#color-structure)
   - [Point Structure](#point-structure)
   - [Size Structure](#size-structure)
   - [Rectangle Structure](#rectangle-structure)
   - [HatchStyle Class](#hatchstyle-class)
   - [Image Class](#image-class)
   - [Graphics Class](#graphics-class)
   - [GraphicsBuffer Class](#graphicsbuffer-class)
4. [System.Devices Namespace](#systemdevices-namespace)
   - [Display Class](#display-class)
   - [Mouse Class](#mouse-class)
   - [Keyboard Class](#keyboard-class)
5. [System.Windows.Forms Namespace](#systemwindowsforms-namespace)
   - [Layout System](#layout-system)
   - [Control Class](#control-class)
   - [Desktop Class](#desktop-class)
   - [Window Class](#window-class)
   - [TaskBar Class](#taskbar-class)
   - [TaskBarButton Class](#taskbarbutton-class)
   - [StartMenu Class](#startmenu-class)
   - [MenuItem Class](#menuitem-class)
   - [Button Class](#button-class)
   - [Picture Class](#picture-class)
   - [SpectrumControl Class](#spectrumcontrol-class)
6. [Platform.DOS Namespace](#platformdos-namespace)

---

# System Namespace

The System namespace contains fundamental classes that define commonly-used value and reference types, events and event handlers, interfaces, attributes, and processing exceptions.

## Primitive Types

WinDOS provides .NET-style wrapper classes for all primitive types. Each wrapper provides:
- `ToString()` - Convert to string representation
- `Parse(const String&)` - Parse from string (throws on failure)
- `TryParse(const String&, T&)` - Safe parse (returns bool)
- `GetHashCode()` - Hash code for collections

### Boolean Class

Represents a Boolean (true or false) value.

```cpp
namespace System {
    class Boolean {
    public:
        // Constants
        static const Boolean True;
        static const Boolean False;

        // Constructors
        Boolean();                    // Default: false
        Boolean(bool value);

        // Conversion
        operator bool() const;

        // Operators
        Boolean operator!() const;
        Boolean operator&&(const Boolean& other) const;
        Boolean operator||(const Boolean& other) const;
        bool operator==(const Boolean& other) const;
        bool operator!=(const Boolean& other) const;

        // Methods
        String ToString() const;      // Returns "True" or "False"
        int GetHashCode() const;

        // Static Methods
        static Boolean Parse(const String& s);
        static bool TryParse(const String& s, Boolean& result);
    };
}
```

**Example:**
```cpp
Boolean flag = true;
Console::WriteLine(flag.ToString());  // Output: True

Boolean parsed;
if (Boolean::TryParse("false", parsed)) {
    Console::WriteLine(parsed.ToString());  // Output: False
}
```

### Char Class

Represents a character as a UTF-16 code unit.

```cpp
namespace System {
    class Char {
    public:
        // Constants
        static constexpr char MinValue = '\0';
        static constexpr char MaxValue = '\x7F';

        // Constructors
        Char();                       // Default: '\0'
        Char(char value);

        // Conversion
        operator char() const;

        // Comparison Operators
        bool operator==(const Char& other) const;
        bool operator!=(const Char& other) const;
        bool operator<(const Char& other) const;
        bool operator>(const Char& other) const;
        bool operator<=(const Char& other) const;
        bool operator>=(const Char& other) const;

        // Instance Methods
        String ToString() const;
        int GetHashCode() const;

        // Static Methods
        static bool IsDigit(char c);
        static bool IsLetter(char c);
        static bool IsLetterOrDigit(char c);
        static bool IsWhiteSpace(char c);
        static bool IsUpper(char c);
        static bool IsLower(char c);
        static char ToUpper(char c);
        static char ToLower(char c);
    };
}
```

**Example:**
```cpp
Char ch = 'A';
Console::WriteLine(ch.ToString());           // Output: A
Console::Write(Char::IsLetter('X'));         // Output: True
Console::Write(Char::ToLower('Z'));          // Output: z
```

### Integer Types

WinDOS provides signed and unsigned integer types of various sizes.

| Type | Underlying | Range |
|------|------------|-------|
| `Int8` / `SByte` | `signed char` | -128 to 127 |
| `UInt8` / `Byte` | `unsigned char` | 0 to 255 |
| `Int16` / `Short` | `short` | -32,768 to 32,767 |
| `UInt16` / `UShort` | `unsigned short` | 0 to 65,535 |
| `Int32` / `Int` | `int` | -2,147,483,648 to 2,147,483,647 |
| `UInt32` / `UInt` | `unsigned int` | 0 to 4,294,967,295 |
| `Int64` / `Long` | `long long` | -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807 |
| `UInt64` / `ULong` | `unsigned long long` | 0 to 18,446,744,073,709,551,615 |

```cpp
namespace System {
    class Int32 {  // Same pattern for all integer types
    public:
        // Constants
        static constexpr int MinValue = -2147483648;
        static constexpr int MaxValue = 2147483647;

        // Constructors
        Int32();                      // Default: 0
        Int32(int value);

        // Conversion
        operator int() const;

        // Arithmetic Operators
        Int32 operator+(const Int32& other) const;
        Int32 operator-(const Int32& other) const;
        Int32 operator*(const Int32& other) const;
        Int32 operator/(const Int32& other) const;  // Throws on divide by zero
        Int32 operator%(const Int32& other) const;
        Int32 operator-() const;      // Unary minus
        Int32 operator+() const;      // Unary plus

        // Increment/Decrement
        Int32& operator++();          // Prefix
        Int32 operator++(int);        // Postfix
        Int32& operator--();
        Int32 operator--(int);

        // Compound Assignment
        Int32& operator+=(const Int32& other);
        Int32& operator-=(const Int32& other);
        Int32& operator*=(const Int32& other);
        Int32& operator/=(const Int32& other);
        Int32& operator%=(const Int32& other);

        // Bitwise Operators
        Int32 operator&(const Int32& other) const;
        Int32 operator|(const Int32& other) const;
        Int32 operator^(const Int32& other) const;
        Int32 operator~() const;
        Int32 operator<<(int shift) const;
        Int32 operator>>(int shift) const;

        // Comparison Operators
        bool operator==(const Int32& other) const;
        bool operator!=(const Int32& other) const;
        bool operator<(const Int32& other) const;
        bool operator>(const Int32& other) const;
        bool operator<=(const Int32& other) const;
        bool operator>=(const Int32& other) const;

        // Methods
        String ToString() const;
        int GetHashCode() const;

        // Static Methods
        static Int32 Parse(const String& s);
        static bool TryParse(const String& s, Int32& result);
    };
}
```

**Example:**
```cpp
Int32 x = 42;
Int32 y = Int32::Parse("100");
Console::WriteLine((x + y).ToString());  // Output: 142

Int32 result;
if (Int32::TryParse("invalid", result)) {
    // Won't reach here
} else {
    Console::WriteLine("Parse failed");
}
```

### Floating-Point Types

| Type | Underlying | Precision |
|------|------------|-----------|
| `Float32` | `float` | ~7 digits |
| `Float64` | `double` | ~15 digits |

```cpp
namespace System {
    class Float64 {  // Same pattern for Float32
    public:
        // Constants
        static constexpr double MinValue;
        static constexpr double MaxValue;
        static constexpr double Epsilon;

        // Constructors
        Float64();                    // Default: 0.0
        Float64(double value);

        // Conversion
        operator double() const;

        // Arithmetic Operators
        Float64 operator+(const Float64& other) const;
        Float64 operator-(const Float64& other) const;
        Float64 operator*(const Float64& other) const;
        Float64 operator/(const Float64& other) const;
        Float64 operator-() const;
        Float64 operator+() const;

        // Compound Assignment
        Float64& operator+=(const Float64& other);
        Float64& operator-=(const Float64& other);
        Float64& operator*=(const Float64& other);
        Float64& operator/=(const Float64& other);

        // Comparison Operators
        bool operator==(const Float64& other) const;
        bool operator!=(const Float64& other) const;
        bool operator<(const Float64& other) const;
        bool operator>(const Float64& other) const;
        bool operator<=(const Float64& other) const;
        bool operator>=(const Float64& other) const;

        // Methods
        String ToString() const;
        int GetHashCode() const;

        // Static Methods
        static bool IsNaN(Float64 value);
        static bool IsInfinity(Float64 value);
        static bool IsPositiveInfinity(Float64 value);
        static bool IsNegativeInfinity(Float64 value);
        static Float64 Parse(const String& s);
        static bool TryParse(const String& s, Float64& result);
    };
}
```

**Example:**
```cpp
Float64 pi = 3.14159;
Console::WriteLine(pi.ToString());           // Output: 3.141590

Float64 inf = 1.0 / 0.0;
if (Float64::IsInfinity(inf)) {
    Console::WriteLine("Is infinite");
}
```

### Math Class

Provides constants and static methods for common mathematical functions and checked arithmetic operations.

```cpp
namespace System {
    class Math {
    public:
        // Basic Operations
        static Int32 Abs(Int32 value);
        static Int32 Min(Int32 a, Int32 b);
        static Int32 Max(Int32 a, Int32 b);
        static Int32 Clamp(Int32 value, Int32 min, Int32 max);
        static void Swap(Int32& a, Int32& b);

        // Checked Arithmetic - throws OverflowException on overflow
        static Int32 CheckedAdd(Int32 a, Int32 b);
        static Int32 CheckedSubtract(Int32 a, Int32 b);
        static Int32 CheckedMultiply(Int32 a, Int32 b);
        static Int32 CheckedCast(size_t value);
        static Int32 CheckedCast(long long value);

        // Safe Arithmetic - returns success/failure instead of throwing
        static bool TryAdd(Int32 a, Int32 b, Int32& result);
        static bool TrySubtract(Int32 a, Int32 b, Int32& result);
        static bool TryMultiply(Int32 a, Int32 b, Int32& result);

    private:
        Math() = delete;  // Static class - no instantiation
    };
}
```

**Example:**
```cpp
// Basic operations
Int32 x = -42;
Console::WriteLine(Math::Abs(x).ToString());     // Output: 42
Console::WriteLine(Math::Min(10, 20).ToString()); // Output: 10
Console::WriteLine(Math::Clamp(150, 0, 100).ToString()); // Output: 100

// Checked arithmetic - throws on overflow
try {
    Int32 huge = Int32::MaxValue;
    Int32 result = Math::CheckedAdd(huge, 1);  // Throws!
} catch (const OverflowException& e) {
    Console::WriteLine("Overflow detected!");
}

// Safe arithmetic - returns success/failure
Int32 a = 1000000;
Int32 b = 1000000;
Int32 product;
if (Math::TryMultiply(a, b, product)) {
    Console::WriteLine(product.ToString());
} else {
    Console::WriteLine("Multiplication would overflow");
}

// Use in string operations (prevents overflow attacks)
Int32 totalLength;
if (!Math::TryAdd(str1.Length(), str2.Length(), totalLength)) {
    throw OverflowException("String too large");
}
```

---

## String Class

Represents text as a sequence of characters. Strings are immutable; operations that appear to modify a string actually return a new String instance.

```cpp
namespace System {
    class String {
    public:
        // Static Members
        static const String Empty;

        // Constructors
        String();                                  // Empty string
        String(const char* s);                     // From C string
        String(const char* s, int length);         // From C string with length
        String(char c, int count);                 // Repeat character
        String(const String& other);               // Copy
        String(String&& other) noexcept;           // Move
        ~String();

        // Assignment
        String& operator=(const String& other);
        String& operator=(String&& other) noexcept;
        String& operator=(const char* s);

        // Properties
        int Length() const;
        bool IsEmpty() const;
        const char* CStr() const;

        // Indexer
        char operator[](int index) const;

        // Substring
        String Substring(int startIndex) const;
        String Substring(int startIndex, int length) const;

        // Search
        int IndexOf(char c) const;
        int IndexOf(char c, int startIndex) const;
        int IndexOf(const String& s) const;
        int IndexOf(const String& s, int startIndex) const;
        int LastIndexOf(char c) const;
        int LastIndexOf(const String& s) const;
        bool Contains(const String& s) const;
        bool StartsWith(const String& s) const;
        bool EndsWith(const String& s) const;

        // Transformation
        String ToUpper() const;
        String ToLower() const;
        String Trim() const;
        String TrimStart() const;
        String TrimEnd() const;
        String Replace(char oldChar, char newChar) const;
        String Replace(const String& oldValue, const String& newValue) const;
        String Insert(int startIndex, const String& value) const;
        String Remove(int startIndex) const;
        String Remove(int startIndex, int count) const;
        String PadLeft(int totalWidth, char paddingChar = ' ') const;
        String PadRight(int totalWidth, char paddingChar = ' ') const;

        // Split
        Array<String> Split(char delimiter) const;
        Array<String> Split(const char* delimiters) const;

        // Comparison
        bool operator==(const String& other) const;
        bool operator!=(const String& other) const;
        bool operator<(const String& other) const;
        bool operator>(const String& other) const;
        bool operator<=(const String& other) const;
        bool operator>=(const String& other) const;
        int CompareTo(const String& other) const;
        bool Equals(const String& other) const;
        bool EqualsIgnoreCase(const String& other) const;

        // Concatenation
        String operator+(const String& other) const;
        String operator+(const char* other) const;
        String operator+(char c) const;
        String& operator+=(const String& other);
        String& operator+=(const char* other);
        String& operator+=(char c);
        friend String operator+(const char* lhs, const String& rhs);

        // Utility
        int GetHashCode() const;

        // Static Methods
        static bool IsNullOrEmpty(const String& s);
        static bool IsNullOrWhiteSpace(const String& s);
        static String Concat(const String& s1, const String& s2);
        static String Concat(const String& s1, const String& s2, const String& s3);
        static int Compare(const String& s1, const String& s2);
        static int CompareIgnoreCase(const String& s1, const String& s2);
    };
}
```

**Example:**
```cpp
String greeting = "Hello, World!";
Console::WriteLine(greeting);                    // Output: Hello, World!
Console::WriteLine(greeting.Length());           // Output: 13
Console::WriteLine(greeting.ToUpper());          // Output: HELLO, WORLD!
Console::WriteLine(greeting.Substring(0, 5));    // Output: Hello
Console::WriteLine(greeting.Contains("World")); // Output: True

// String concatenation
String name = "DOS";
String message = "Welcome to " + name + "!";
Console::WriteLine(message);                     // Output: Welcome to DOS!

// Split
String csv = "one,two,three";
Array<String> parts = csv.Split(',');
for (int i = 0; i < parts.Length(); i++) {
    Console::WriteLine(parts[i]);
}
// Output:
// one
// two
// three

// Trim
String padded = "  spaced  ";
Console::WriteLine(padded.Trim());              // Output: spaced
```

---

## StringBuilder Class

Represents a mutable string of characters with efficient append operations. Use StringBuilder when concatenating many strings in a loop or building strings incrementally.

```cpp
namespace System {
    class StringBuilder {
    public:
        // Constructors
        StringBuilder();
        explicit StringBuilder(Int32 capacity);
        explicit StringBuilder(const String& value);
        StringBuilder(const StringBuilder& other);
        StringBuilder(StringBuilder&& other) noexcept;
        ~StringBuilder();

        // Properties
        Int32 Length() const;
        Int32 Capacity() const;
        Char operator[](Int32 index) const;
        void SetCharAt(Int32 index, Char c);

        // Append methods (return *this for chaining)
        StringBuilder& Append(const String& value);
        StringBuilder& Append(const char* value);
        StringBuilder& Append(Char value);
        StringBuilder& Append(char value);
        StringBuilder& Append(Int32 value);
        StringBuilder& Append(bool value);
        StringBuilder& AppendLine();
        StringBuilder& AppendLine(const String& value);
        StringBuilder& AppendLine(const char* value);

        // Insert
        StringBuilder& Insert(Int32 index, const String& value);
        StringBuilder& Insert(Int32 index, const char* value);
        StringBuilder& Insert(Int32 index, Char value);

        // Remove
        StringBuilder& Remove(Int32 startIndex, Int32 length);

        // Clear and Reserve
        StringBuilder& Clear();
        void Reserve(Int32 capacity);

        // Convert to String
        String ToString() const;
    };
}
```

**Example:**
```cpp
// Building a string efficiently in a loop
StringBuilder sb;
for (int i = 0; i < 100; i++) {
    sb.Append("Line ").Append(Int32(i)).AppendLine();
}
String result = sb.ToString();

// Method chaining
StringBuilder builder;
builder.Append("Hello")
       .Append(' ')
       .Append("World")
       .Append('!');
String greeting = builder.ToString();  // "Hello World!"

// With initial capacity (avoids reallocations)
StringBuilder large(1000);
large.Append("Large text...");
```

---

## MemoryPool Class

A fixed-size block memory pool for efficient small allocations. Reduces heap fragmentation and allocation overhead when allocating many objects of the same size.

```cpp
namespace System {
    class MemoryPool {
    public:
        // Constructor
        MemoryPool(Int32 blockSize, Int32 blockCount);
        ~MemoryPool();

        // Non-copyable, movable
        MemoryPool(MemoryPool&& other) noexcept;
        MemoryPool& operator=(MemoryPool&& other) noexcept;

        // Allocation
        void* Allocate();         // Returns nullptr if pool exhausted
        void Free(void* ptr);     // Return block to pool
        void Reset();             // Make all blocks available

        // Properties
        Int32 BlockSize() const;
        Int32 BlockCount() const;
        Int32 FreeCount() const;
        Int32 UsedCount() const;
        Boolean IsEmpty() const;  // No free blocks
        Boolean IsFull() const;   // All blocks free
    };
}
```

**Example:**
```cpp
// Create a pool for 100 MyObject instances
MemoryPool pool(sizeof(MyObject), 100);

// Allocate from pool (faster than new)
void* ptr = pool.Allocate();
if (ptr) {
    MyObject* obj = new(ptr) MyObject();  // Placement new

    // Use object...

    obj->~MyObject();  // Manual destructor call
    pool.Free(ptr);    // Return to pool
}

// Check pool status
Console::WriteLine(pool.FreeCount().ToString());  // Remaining blocks
Console::WriteLine(pool.UsedCount().ToString());  // Allocated blocks

// Reset pool (invalidates all pointers!)
pool.Reset();
```

---

## StringIntern Class

Provides string interning for memory-efficient string storage. Equal strings share the same memory, enabling O(1) pointer comparison instead of O(n) value comparison.

```cpp
namespace System {
    class StringIntern {
    public:
        // Intern a string (returns canonical pointer)
        static const char* Intern(const char* str);
        static const char* Intern(const char* str, Int32 length);
        static const char* Intern(const String& str);

        // Check if string is interned
        static Boolean IsInterned(const char* str);

        // Pool statistics
        static Int32 Count();

        // Pre-interned common strings
        static const char* True();
        static const char* False();
        static const char* Empty();
        static const char* Null();
        static const char* NewLine();
    };
}
```

**Example:**
```cpp
// Intern strings for fast comparison
const char* s1 = StringIntern::Intern("Hello");
const char* s2 = StringIntern::Intern("Hello");

// Same pointer - O(1) comparison!
if (s1 == s2) {
    Console::WriteLine("Strings are equal (same pointer)");
}

// Use pre-interned strings
const char* trueStr = StringIntern::True();   // "True"
const char* falseStr = StringIntern::False(); // "False"

// Check if a string is interned
if (StringIntern::IsInterned("Hello")) {
    Console::WriteLine("Already in pool");
}

// Get pool size
Console::WriteLine(StringIntern::Count().ToString());
```

**When to Use String Interning:**
- Comparing strings frequently (identifiers, keywords)
- Storing many duplicate strings (parsed data, configs)
- Memory-constrained environments

**When NOT to Use:**
- Dynamically generated strings in loops (leaks memory)
- Strings used only once

---

## Array Class

Provides methods for creating, manipulating, searching, and sorting arrays. Array<T> is a generic template with bounds checking.

```cpp
namespace System {
    template<typename T>
    class Array {
    public:
        // Constructors
        Array();                                   // Empty array
        explicit Array(int length);                // Sized array (value-initialized)
        Array(std::initializer_list<T> init);      // Initializer list
        Array(const Array& other);                 // Copy
        Array(Array&& other) noexcept;             // Move
        ~Array();

        // Assignment
        Array& operator=(const Array& other);
        Array& operator=(Array&& other) noexcept;
        Array& operator=(std::initializer_list<T> init);

        // Properties
        int Length() const;
        bool IsEmpty() const;

        // Element Access (throws IndexOutOfRangeException)
        T& operator[](int index);
        const T& operator[](int index) const;
        T& GetValue(int index);
        const T& GetValue(int index) const;
        void SetValue(int index, const T& value);

        // Raw Access
        T* Data();
        const T* Data() const;

        // Iteration
        T* begin();
        T* end();
        const T* begin() const;
        const T* end() const;

        // Modification
        void Clear();                              // Set all to default
        void Resize(int newLength);                // Resize array
        void CopyTo(Array<T>& destination, int destinationIndex) const;
        void Reverse();                            // Reverse in place

        // Search
        int IndexOf(const T& value) const;         // Returns -1 if not found
        bool Contains(const T& value) const;

        // Static Methods
        static Array<T> FromPointer(const T* data, int length);
    };
}
```

**Example:**
```cpp
// Create and initialize
Array<Int32> numbers = {1, 2, 3, 4, 5};
Console::WriteLine(numbers.Length());            // Output: 5

// Access elements
Console::WriteLine(numbers[2].ToString());       // Output: 3

// Bounds checking
try {
    int x = numbers[10];  // Throws IndexOutOfRangeException
} catch (const IndexOutOfRangeException& e) {
    Console::WriteLine(e.Message());
}

// Range-based for loop
for (const auto& num : numbers) {
    Console::Write(num.ToString() + " ");
}
// Output: 1 2 3 4 5

// Search
if (numbers.Contains(3)) {
    Console::WriteLine("Found 3 at index " +
        Int32(numbers.IndexOf(3)).ToString());
}

// Resize
numbers.Resize(10);
Console::WriteLine(numbers.Length());            // Output: 10

// Reverse
numbers.Reverse();
```

---

## Exception Classes

WinDOS provides a hierarchy of exception classes for error handling, modeled after .NET's exception system.

### Exception Hierarchy

```
Exception (base)
├── ArgumentException
│   ├── ArgumentNullException
│   └── ArgumentOutOfRangeException
├── InvalidOperationException
├── IndexOutOfRangeException
├── NullReferenceException
├── FormatException
├── OverflowException
├── IOException
│   └── FileNotFoundException
└── InvalidDataException
```

### Exception (Base Class)

```cpp
namespace System {
    class Exception {
    public:
        Exception();
        Exception(const char* message);
        Exception(const Exception& other);
        Exception& operator=(const Exception& other);
        virtual ~Exception();

        virtual const char* Message() const;
        virtual const char* what() const noexcept;  // STL compatibility
    };
}
```

### ArgumentException

Thrown when an argument to a method is invalid.

```cpp
namespace System {
    class ArgumentException : public Exception {
    public:
        ArgumentException(const char* message, const char* paramName = nullptr);
        const char* ParamName() const;
    };
}
```

### ArgumentNullException

Thrown when a null reference is passed to a method that does not accept it.

```cpp
namespace System {
    class ArgumentNullException : public ArgumentException {
    public:
        explicit ArgumentNullException(const char* paramName);
    };
}
```

### ArgumentOutOfRangeException

Thrown when an argument is outside the allowable range.

```cpp
namespace System {
    class ArgumentOutOfRangeException : public ArgumentException {
    public:
        ArgumentOutOfRangeException(const char* paramName,
                                     const char* message = nullptr);
    };
}
```

### InvalidOperationException

Thrown when a method call is invalid for the object's current state.

```cpp
namespace System {
    class InvalidOperationException : public Exception {
    public:
        explicit InvalidOperationException(const char* message);
    };
}
```

### IndexOutOfRangeException

Thrown when an attempt is made to access an element with an invalid index.

```cpp
namespace System {
    class IndexOutOfRangeException : public Exception {
    public:
        IndexOutOfRangeException();
        explicit IndexOutOfRangeException(const char* message);
    };
}
```

### NullReferenceException

Thrown when there is an attempt to dereference a null object reference.

```cpp
namespace System {
    class NullReferenceException : public Exception {
    public:
        NullReferenceException();
        explicit NullReferenceException(const char* message);
    };
}
```

### FormatException

Thrown when the format of an argument is invalid.

```cpp
namespace System {
    class FormatException : public Exception {
    public:
        FormatException();
        explicit FormatException(const char* message);
    };
}
```

### OverflowException

Thrown when an arithmetic, casting, or conversion operation results in an overflow.

```cpp
namespace System {
    class OverflowException : public Exception {
    public:
        OverflowException();
        explicit OverflowException(const char* message);
    };
}
```

### IOException

Thrown when an I/O error occurs.

```cpp
namespace System {
    class IOException : public Exception {
    public:
        IOException();
        explicit IOException(const char* message);
    };
}
```

### FileNotFoundException

Thrown when an attempt to access a file that does not exist fails.

```cpp
namespace System {
    class FileNotFoundException : public IOException {
    public:
        FileNotFoundException();
        explicit FileNotFoundException(const char* path);
    };
}
```

### InvalidDataException

Thrown when data is in an invalid format.

```cpp
namespace System {
    class InvalidDataException : public Exception {
    public:
        InvalidDataException();
        explicit InvalidDataException(const char* message);
    };
}
```

**Example:**
```cpp
void ProcessData(const char* data, int length) {
    if (data == nullptr) {
        throw ArgumentNullException("data");
    }
    if (length < 0) {
        throw ArgumentOutOfRangeException("length", "Length cannot be negative");
    }
    // Process...
}

try {
    ProcessData(nullptr, 10);
} catch (const ArgumentNullException& e) {
    Console::WriteLine(String("Error: ") + e.Message());
    Console::WriteLine(String("Parameter: ") + e.ParamName());
}
```

---

## Console Class

Represents the standard input, output, and error streams for console applications. Provides methods for reading from and writing to the console with color support.

### ConsoleColor Enumeration

```cpp
namespace System {
    enum class ConsoleColor : unsigned char {
        Black       = 0,
        DarkBlue    = 1,
        DarkGreen   = 2,
        DarkCyan    = 3,
        DarkRed     = 4,
        DarkMagenta = 5,
        DarkYellow  = 6,   // Brown
        Gray        = 7,
        DarkGray    = 8,
        Blue        = 9,
        Green       = 10,
        Cyan        = 11,
        Red         = 12,
        Magenta     = 13,
        Yellow      = 14,
        White       = 15
    };
}
```

### Console Class

```cpp
namespace System {
    class Console {
    public:
        // Output (no newline)
        static void Write(const String& value);
        static void Write(const char* value);
        static void Write(char value);
        static void Write(bool value);
        static void Write(int value);
        static void Write(unsigned int value);
        static void Write(long long value);
        static void Write(unsigned long long value);
        static void Write(float value);
        static void Write(double value);

        // Output (with newline)
        static void WriteLine();
        static void WriteLine(const String& value);
        // ... same overloads as Write ...

        // Input
        static String ReadLine();
        static Char ReadKey();
        static Char ReadKey(bool intercept);  // intercept=true suppresses echo
        static Boolean KeyAvailable();

        // Cursor
        static void SetCursorPosition(Int32 left, Int32 top);
        static Int32 CursorLeft();
        static Int32 CursorTop();

        // Colors
        static ConsoleColor ForegroundColor();
        static void SetForegroundColor(ConsoleColor color);
        static ConsoleColor BackgroundColor();
        static void SetBackgroundColor(ConsoleColor color);
        static void ResetColor();

        // Screen
        static void Clear();
        static Int32 WindowWidth();
        static Int32 WindowHeight();

        // Audio
        static void Beep();
    };
}
```

**Example:**
```cpp
// Basic output
Console::WriteLine("Hello, DOS!");
Console::Write("Enter your name: ");
String name = Console::ReadLine();
Console::WriteLine("Hello, " + name + "!");

// Colors
Console::SetForegroundColor(ConsoleColor::Green);
Console::WriteLine("Success!");
Console::SetForegroundColor(ConsoleColor::Red);
Console::WriteLine("Error!");
Console::ResetColor();

// Cursor positioning
Console::Clear();
Console::SetCursorPosition(10, 5);
Console::WriteLine("Centered-ish text");

// Key input
Console::Write("Press any key...");
Char key = Console::ReadKey(true);  // Don't echo
Console::WriteLine();
Console::WriteLine("You pressed: " + key.ToString());

// Check for key without blocking
while (!Console::KeyAvailable()) {
    // Do work...
}
```

---

# System.IO Namespace

The System.IO namespace contains types for reading and writing to files.

## File Class

Provides static methods for reading files.

```cpp
namespace System::IO {
    class File {
    public:
        // Read entire file into byte array
        // Throws: FileNotFoundException, IOException
        static Array<UInt8> ReadAllBytes(const char* path);

        // Check if file exists
        static Boolean Exists(const char* path);

        // Get file size in bytes
        // Throws: FileNotFoundException
        static Int64 GetSize(const char* path);

    private:
        File() = delete;  // Static class - no instantiation
    };
}
```

**Example:**
```cpp
#include "System/IO/IO.hpp"

using namespace System;
using namespace System::IO;

// Check if file exists
if (File::Exists("config.dat")) {
    // Read file contents
    Array<UInt8> data = File::ReadAllBytes("config.dat");
    Console::WriteLine("Read " + Int32(data.Length()).ToString() + " bytes");
}

// Get file size without reading
try {
    Int64 size = File::GetSize("largefile.bin");
    Console::WriteLine("File is " + size.ToString() + " bytes");
} catch (const FileNotFoundException& e) {
    Console::WriteLine("File not found!");
}

// Error handling
try {
    Array<UInt8> data = File::ReadAllBytes("missing.txt");
} catch (const FileNotFoundException& e) {
    Console::WriteLine(e.Message());
} catch (const IOException& e) {
    Console::WriteLine("I/O error: " + String(e.Message()));
}
```

---

# System.Drawing Namespace

The System.Drawing namespace provides access to GDI+ basic graphics functionality. It contains classes for drawing shapes, displaying images, and managing colors.

## Color Structure

Represents a 32-bit ARGB color. Unified color representation used throughout the graphics system. When rendering to lower color depth displays (4bpp/8bpp VGA), Bayer dithering is applied automatically.

```cpp
namespace System::Drawing {
    class Color {
    public:
        // Constructors
        Color();                                      // Default (Black, fully opaque)
        Color(UInt8 r, UInt8 g, UInt8 b);            // RGB with full alpha (255)
        Color(UInt8 a, UInt8 r, UInt8 g, UInt8 b);   // Full ARGB
        Color(unsigned int argb);                     // From packed 0xAARRGGBB
        Color(const Color& other);

        // Component Accessors
        UInt8 A() const;                              // Alpha (0-255)
        UInt8 R() const;                              // Red (0-255)
        UInt8 G() const;                              // Green (0-255)
        UInt8 B() const;                              // Blue (0-255)
        unsigned int ToArgb() const;                  // Packed 0xAARRGGBB

        // Operators
        Color& operator=(const Color& other);
        bool operator==(const Color& other) const;
        bool operator!=(const Color& other) const;

        // Color Interpolation
        static Color Lerp(const Color& c1, const Color& c2, float t);

        // VGA Palette Mapping (for dithering)
        static UInt8 RgbToVgaIndex(UInt8 r, UInt8 g, UInt8 b);

        // Named Colors (32-bit ARGB)
        static const Color Black;         // 0xFF000000
        static const Color DarkBlue;      // 0xFF000080
        static const Color DarkGreen;     // 0xFF008000
        static const Color DarkCyan;      // 0xFF008080
        static const Color DarkRed;       // 0xFF800000
        static const Color DarkMagenta;   // 0xFF800080
        static const Color DarkYellow;    // 0xFF808000 (Brown)
        static const Color Gray;          // 0xFFC0C0C0
        static const Color DarkGray;      // 0xFF808080
        static const Color Blue;          // 0xFF0000FF
        static const Color Green;         // 0xFF00FF00
        static const Color Cyan;          // 0xFF00FFFF
        static const Color Red;           // 0xFFFF0000
        static const Color Magenta;       // 0xFFFF00FF
        static const Color Yellow;        // 0xFFFFFF00
        static const Color White;         // 0xFFFFFFFF
        static const Color Transparent;   // 0x00000000
    };

    // Backwards compatibility alias
    typedef Color Color32;
}
```

**Example:**
```cpp
// Create colors
Color red(255, 0, 0);                  // RGB red
Color semiTransparent(128, 0, 255, 0); // 50% transparent green
Color fromHex(0xFF00BFFF);             // Deep sky blue

// Access components
UInt8 alpha = semiTransparent.A();     // 128
UInt8 green = semiTransparent.G();     // 255

// Interpolate colors (for gradients)
Color start = Color::White;
Color end = Color::Blue;
Color mid = Color::Lerp(start, end, 0.5f);  // Halfway between

// Use named colors
Color background = Color::Cyan;
Color foreground = Color::White;

// Compare colors
if (background == Color::Cyan) {
    Console::WriteLine("Background is cyan");
}
```

---

## Point Structure

Represents an ordered pair of integer x- and y-coordinates.

```cpp
namespace System::Drawing {
    struct Point {
        int x, y;

        // Constructors
        Point();                          // (0, 0)
        Point(int x, int y);
        Point(const Point& other);

        // Operators
        Point& operator=(const Point& other);
        bool operator==(const Point& other) const;
        bool operator!=(const Point& other) const;

        // Methods
        Point Offset(int dx, int dy) const;

        // Static Members
        static const Point Empty;         // (0, 0)
    };
}
```

**Example:**
```cpp
Point location(100, 50);
Point moved = location.Offset(10, 20);  // (110, 70)
```

---

## Size Structure

Stores an ordered pair of integers representing width and height.

```cpp
namespace System::Drawing {
    struct Size {
        int width, height;

        // Constructors
        Size();                           // (0, 0)
        Size(int width, int height);
        Size(const Size& other);

        // Operators
        Size& operator=(const Size& other);
        bool operator==(const Size& other) const;
        bool operator!=(const Size& other) const;

        // Methods
        bool IsEmpty() const;             // width==0 || height==0

        // Static Members
        static const Size Empty;

        // Standard Icon Sizes
        static const Size IconSmall;      // 16x16
        static const Size IconCursor;     // 24x24 (cursor size)
        static const Size IconMedium;     // 32x32 (desktop icons)
        static const Size IconLarge;      // 48x48
    };
}
```

**Example:**
```cpp
// Use standard icon sizes
Image smallIcon = Image::FromIcon("app.ico", Size::IconSmall);   // 16x16
Image desktopIcon = Image::FromIcon("app.ico", Size::IconMedium); // 32x32
Image largeIcon = Image::FromIcon("app.ico", Size::IconLarge);    // 48x48

// Custom size
Size customSize(64, 64);
```

---

## Rectangle Structure

Stores a set of four integers that represent the location and size of a rectangle.

```cpp
namespace System::Drawing {
    struct Rectangle {
        int x, y, width, height;

        // Constructors
        Rectangle();                                      // All zeros
        Rectangle(int x, int y, int width, int height);
        Rectangle(const Point& location, const Size& size);
        Rectangle(const Rectangle& other);

        // Operators
        Rectangle& operator=(const Rectangle& other);

        // Properties
        Point Location() const;
        Size GetSize() const;
        int Left() const;                 // x
        int Top() const;                  // y
        int Right() const;                // x + width
        int Bottom() const;               // y + height

        // Methods
        bool Contains(int px, int py) const;
        bool Contains(const Point& pt) const;
        Rectangle Offset(int dx, int dy) const;
        Rectangle Inflate(int dx, int dy) const;

        // Static Members
        static const Rectangle Empty;
    };
}
```

**Example:**
```cpp
Rectangle bounds(10, 20, 100, 50);

// Check if point is inside
Point click(50, 30);
if (bounds.Contains(click)) {
    Console::WriteLine("Click inside rectangle");
}

// Get edges
Console::WriteLine(bounds.Left());    // 10
Console::WriteLine(bounds.Right());   // 110
Console::WriteLine(bounds.Top());     // 20
Console::WriteLine(bounds.Bottom());  // 70
```

---

## HatchStyle Class

Provides predefined 8x8 pixel patterns for pattern fills. Each pattern tiles seamlessly across any rectangle.

```cpp
namespace System::Drawing {
    class HatchStyle {
    public:
        // Pattern query
        bool GetBit(int x, int y) const;  // Returns true for foreground pixel

        // Solid fills
        static const HatchStyle Solid;           // All foreground
        static const HatchStyle Empty;           // All background

        // Lines
        static const HatchStyle Horizontal;      // Horizontal lines
        static const HatchStyle Vertical;        // Vertical lines
        static const HatchStyle Cross;           // Grid (horizontal + vertical)
        static const HatchStyle ForwardDiagonal; // /// diagonal lines
        static const HatchStyle BackwardDiagonal;// \\\ diagonal lines
        static const HatchStyle DiagonalCross;   // X pattern

        // Light/Dark variants
        static const HatchStyle LightHorizontal;
        static const HatchStyle LightVertical;
        static const HatchStyle DarkHorizontal;
        static const HatchStyle DarkVertical;

        // Dashed
        static const HatchStyle DashedHorizontal;
        static const HatchStyle DashedVertical;

        // Percentage fills (dot density)
        static const HatchStyle Percent05;
        static const HatchStyle Percent10;
        static const HatchStyle Percent20;
        static const HatchStyle Percent25;
        static const HatchStyle Percent30;
        static const HatchStyle Percent40;
        static const HatchStyle Percent50;       // Checkerboard
        static const HatchStyle Percent60;
        static const HatchStyle Percent70;
        static const HatchStyle Percent75;
        static const HatchStyle Percent80;
        static const HatchStyle Percent90;

        // Grid patterns
        static const HatchStyle SmallGrid;
        static const HatchStyle LargeGrid;
        static const HatchStyle DottedGrid;

        // Special textures
        static const HatchStyle DottedDiamond;
        static const HatchStyle Brick;
        static const HatchStyle Weave;
        static const HatchStyle Trellis;
        static const HatchStyle Sphere;
        static const HatchStyle Wave;
        static const HatchStyle ZigZag;
        static const HatchStyle Shingle;
        static const HatchStyle Plaid;

    private:
        HatchStyle(const unsigned char pattern[8]);
        unsigned char _pattern[8];  // 8x8 bitmap (1 bit per pixel)
    };
}
```

**Example:**
```cpp
Graphics gfx(BufferMode::Double, 0, 0, 640, 480);

// Fill with solid color
gfx.FillRectangle(10, 10, 100, 100, Color::Blue);

// Fill with pattern
gfx.FillRectangle(120, 10, 100, 100,
    HatchStyle::DiagonalCross, Color::Red, Color::White);

// Brick pattern for wall texture
gfx.FillRectangle(230, 10, 100, 100,
    HatchStyle::Brick, Color::DarkRed, Color::Gray);

// Percentage fill for shading
gfx.FillRectangle(10, 120, 100, 100,
    HatchStyle::Percent50, Color::Black, Color::White);

// Transparent background (only foreground pixels drawn)
gfx.FillRectangle(120, 120, 100, 100,
    HatchStyle::Cross, Color::Green, Color::Transparent);
```

---

## Image Class

Represents an image with 32-bit ARGB pixel data. Supports loading from BMP files and Windows icon resources.

```cpp
namespace System::Drawing {
    class Image {
    public:
        // Constructors
        Image();                                          // Empty image
        Image(Int32 width, Int32 height,
              const Color& fillColor = Color::Black);
        Image(const Size& size,
              const Color& fillColor = Color::Black);
        Image(const Image& other);                        // Copy
        Image(Image&& other) noexcept;                    // Move
        ~Image();

        // Assignment
        Image& operator=(const Image& other);
        Image& operator=(Image&& other) noexcept;

        // Properties
        Int32 Width() const;
        Int32 Height() const;
        Size GetSize() const;
        Int32 Length() const;                             // width * height
        unsigned int* Data();                             // Raw 32-bit ARGB pixels
        const unsigned int* Data() const;

        // Pixel Operations
        Color GetPixel(Int32 x, Int32 y) const;
        void SetPixel(Int32 x, Int32 y, const Color& color);
        void SetPixel(const Point& pt, const Color& color);

        // Image Operations
        void Clear(const Color& color);
        void CopyFrom(const Image& src, Int32 destX, Int32 destY);
        void CopyFrom(const Image& src, const Point& dest);
        // Copy with transparency - pixels matching transparentColor are skipped
        void CopyFrom(const Image& src, Int32 destX, Int32 destY,
                      const Color& transparentColor);
        // Copy with alpha blending
        void CopyFromWithAlpha(const Image& src, Int32 destX, Int32 destY);
        Image GetRegion(Int32 x, Int32 y, Int32 width, Int32 height) const;
        Image GetRegion(const Rectangle& rect) const;

        // Static Methods - BMP Loading
        // Loads BMP files (4bpp, 8bpp, 24bpp, 32bpp supported)
        // Throws: FileNotFoundException, InvalidDataException
        static Image FromBitmap(const char* path);

        // Static Methods - Icon Loading
        // Load standalone .ico file at specified size
        // Throws: FileNotFoundException, InvalidDataException, ArgumentException
        static Image FromIcon(const char* path, const Size& size);

        // Load icon from PE executable (.dll, .exe, .icl)
        // Throws: FileNotFoundException, InvalidDataException, ArgumentException
        static Image FromIconLibrary(const char* path, Int32 iconIndex,
                                     const Size& size);

        // Get number of icons in a PE icon library
        // Throws: FileNotFoundException, InvalidDataException
        static Int32 GetIconLibraryCount(const char* path);
    };

    // Standard icon sizes (use with FromIcon/FromIconLibrary)
    // Size::IconSmall   - 16x16
    // Size::IconCursor  - 24x24
    // Size::IconMedium  - 32x32
    // Size::IconLarge   - 48x48

    // Backwards compatibility alias
    typedef Image Image32;
}
```

**Example:**
```cpp
// Create a blank image
Image canvas(320, 200, Color::Black);

// Draw pixels
canvas.SetPixel(100, 100, Color::White);
canvas.SetPixel(Point(101, 100), Color::Red);

// Load from BMP file (auto-detects 4/8/24/32bpp)
try {
    Image background = Image::FromBitmap("assets\\bg.bmp");
} catch (const FileNotFoundException& e) {
    Console::WriteLine("File not found!");
} catch (const InvalidDataException& e) {
    Console::WriteLine("Invalid BMP format!");
}

// Copy region
Image region = background.GetRegion(0, 0, 64, 64);

// Copy with transparency (magenta = transparent)
Image sprite = Image::FromBitmap("assets\\sprite.bmp");
canvas.CopyFrom(sprite, 50, 50, Color::Magenta);  // Magenta pixels not copied

// Copy with alpha blending
Image overlay = Image::FromBitmap("assets\\overlay.bmp");
canvas.CopyFromWithAlpha(overlay, 100, 100);

// Load standalone .ico file
Image icon = Image::FromIcon("myicon.ico", Size::IconMedium);  // 32x32

// Load icons from Windows DLL
Int32 iconCount = Image::GetIconLibraryCount("shell32.dll");
Console::WriteLine("Found " + iconCount.ToString() + " icons");

Image folderIcon = Image::FromIconLibrary("shell32.dll", 3, Size::IconMedium);
Image computerIcon = Image::FromIconLibrary("shell32.dll", 15, Size::IconLarge);

// Clear
canvas.Clear(Color::Blue);
```

---

## Graphics Class

Encapsulates a drawing surface.

```cpp
namespace System::Drawing {
    enum class BufferMode {
        Single,       // Direct rendering
        Double        // Double-buffered
    };

    enum class BorderStyle {
        None,
        Flat,
        Raised,
        Sunken,
        RaisedDouble,
        SunkenDouble,
        Window
    };

    class Graphics {
    public:
        // Constructors
        Graphics(BufferMode mode, const Rectangle& bounds);
        Graphics(BufferMode mode, int x, int y, int width, int height);
        ~Graphics();

        // Properties
        const Rectangle& Bounds() const;

        // Drawing Methods
        void Clear(const Color& color);
        void DrawPixel(int x, int y, const Color& color);
        void DrawPixel(const Point& pt, const Color& color);
        void DrawLine(int x1, int y1, int x2, int y2, const Color& color);
        void DrawLine(const Point& p1, const Point& p2, const Color& color);
        void DrawRectangle(int x, int y, int width, int height,
                          const Color& color);
        void DrawRectangle(const Rectangle& rect, const Color& color);
        void FillRectangle(Int32 x, Int32 y, Int32 width, Int32 height,
                          const Color& color);
        void FillRectangle(const Rectangle& rect, const Color& color);
        void FillRectangle(const Rectangle& rect, BorderStyle style);

        // Pattern fills with hatch styles
        void FillRectangle(Int32 x, Int32 y, Int32 width, Int32 height,
                          const HatchStyle& hatch,
                          const Color& foreColor, const Color& backColor);
        void FillRectangle(const Rectangle& rect, const HatchStyle& hatch,
                          const Color& foreColor, const Color& backColor);

        void DrawImage(const Image& image, Int32 x, Int32 y);
        void DrawImage(const Image& image, const Point& location);

        // Buffer Management
        void Invalidate(bool flushFrameBuffer = false);
    };
}
```

**Example:**
```cpp
// Create graphics context
Graphics gfx(BufferMode::Double, 0, 0, 640, 480);

// Draw shapes
gfx.Clear(Color::Black);
gfx.DrawLine(0, 0, 639, 479, Color::White);
gfx.DrawRectangle(50, 50, 100, 100, Color::Red);
gfx.FillRectangle(200, 50, 100, 100, Color::Green);

// Draw with border style
gfx.FillRectangle(Rectangle(50, 200, 100, 30), BorderStyle::Raised);

// Draw with hatch pattern
gfx.FillRectangle(350, 50, 100, 100,
    HatchStyle::DiagonalCross, Color::Blue, Color::White);

// Draw image
Image sprite = Image::FromBitmap("sprite.bmp");
gfx.DrawImage(sprite, 300, 200);

// Commit to screen
gfx.Invalidate(true);
```

---

## GraphicsBuffer Class

Manages frame buffers for rendering.

```cpp
namespace System::Drawing {
    class GraphicsBuffer {
    public:
        // Properties
        const Rectangle& Bounds() const;
        Image& GetImage();
        const Image& GetImage() const;

        // Methods
        void Invalidate();                // Mark for redraw
        ~GraphicsBuffer();

        // Static Methods (Frame Buffer Management)
        static void CreateFrameBuffer(int width, int height,
                                      unsigned char videoMode);
        static void DestroyFrameBuffer();
        static void FlushFrameBuffer();   // Commit to hardware
        static GraphicsBuffer* GetFrameBuffer();
        static GraphicsBuffer* Create(BufferMode mode,
                                      const Rectangle& bounds);
    };
}
```

**Example:**
```cpp
// Initialize frame buffer for VGA mode 0x12 (640x480x4)
GraphicsBuffer::CreateFrameBuffer(640, 480, 0x12);

// Get the frame buffer
GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
Image& screen = fb->GetImage();

// Draw directly to buffer
screen.SetPixel(320, 240, Color::White);

// Commit to screen
GraphicsBuffer::FlushFrameBuffer();

// Cleanup
GraphicsBuffer::DestroyFrameBuffer();
```

---

# System.Devices Namespace

The System.Devices namespace provides classes for interacting with hardware devices.

## Display Class

Represents display settings and provides methods for changing video modes, including palette fade effects.

```cpp
namespace System::Devices {
    class Display {
    public:
        // Constructors
        Display(const Display& other);
        Display& operator=(const Display& other);

        // Properties (read-only)
        unsigned char Mode() const;
        unsigned char BitsPerPixel() const;
        unsigned short Width() const;
        unsigned short Height() const;

        // Static Methods - Mode Control
        static Display GetCurrent();
        static void SetMode(const Display& display);
        static void SetDefaultMode();     // Text mode
        static void WaitForVSync();

        // Static Methods - VBE High-Resolution
        static Boolean IsVbeAvailable();
        static Display DetectVbeMode(Int32 minWidth, Int32 minHeight);

        // Static Methods - Palette Fade Effects
        // Smoothly fades screen from black to current contents
        static void FadeIn(Int32 milliseconds);
        // Smoothly fades screen from current contents to black
        static void FadeOut(Int32 milliseconds);

        // Static Display Constants
        static const Display TextMode;       // 0x03: 80x25 text
        static const Display VGA_320x200x8;  // 0x13: 320x200, 256 colors
        static const Display VGA_640x480x4;  // 0x12: 640x480, 16 colors
    };
}
```

**Example:**
```cpp
// Switch to graphics mode
Display::SetMode(Display::VGA_640x480x4);

// Get current mode info
Display current = Display::GetCurrent();
Console::WriteLine(current.Width());   // 640
Console::WriteLine(current.Height());  // 480

// Wait for vertical sync (reduces tearing)
Display::WaitForVSync();

// Fade effects for transitions
Display::FadeOut(500);    // Fade to black over 500ms
// ... change scene ...
Display::FadeIn(500);     // Fade in from black over 500ms

// Check for VBE high-resolution support
if (Display::IsVbeAvailable()) {
    Display vbeMode = Display::DetectVbeMode(800, 600);
    Display::SetMode(vbeMode);
}

// Return to text mode
Display::SetDefaultMode();
```

---

## Mouse Class

Provides methods for interacting with the mouse.

### MouseStatus Structure

```cpp
namespace System::Devices {
    struct MouseStatus {
        int x, y;
        bool leftButton;
        bool rightButton;
        bool middleButton;

        MouseStatus();
        MouseStatus(int x, int y, bool left, bool right, bool middle);
    };
}
```

### Mouse Class

```cpp
namespace System::Devices {
    class Mouse {
    public:
        // Initialization
        static bool Initialize();
        static bool IsAvailable();

        // Cursor
        static void ShowCursor();
        static void HideCursor();

        // Status
        static MouseStatus GetStatus();
        static int GetX();
        static int GetY();

        // Control
        static void SetPosition(int x, int y);
        static void SetBounds(int minX, int minY, int maxX, int maxY);

        // Buttons
        static bool IsLeftButtonPressed();
        static bool IsRightButtonPressed();
    };
}
```

**Example:**
```cpp
// Initialize mouse
if (!Mouse::Initialize()) {
    Console::WriteLine("Mouse not found!");
    return 1;
}

// Set movement bounds
Mouse::SetBounds(0, 0, 639, 479);

// Hide hardware cursor (use software cursor)
Mouse::HideCursor();

// Main loop
while (true) {
    MouseStatus status = Mouse::GetStatus();

    // Draw cursor at status.x, status.y

    if (status.leftButton) {
        // Handle left click at (status.x, status.y)
    }

    if (status.rightButton) {
        break;  // Exit on right click
    }
}
```

---

## Keyboard Class

Provides methods for keyboard input.

### KeyboardStatus Structure

```cpp
namespace System::Devices {
    struct KeyboardStatus {
        bool shiftPressed;
        bool ctrlPressed;
        bool altPressed;
        bool capsLock;
        bool numLock;
        bool scrollLock;

        KeyboardStatus();
    };
}
```

### Keyboard Class

```cpp
namespace System::Devices {
    class Keyboard {
    public:
        // Status
        static bool IsKeyPressed();
        static char ReadKey();            // Blocking read
        static char PeekKey();            // Non-blocking peek
        static KeyboardStatus GetStatus();
    };
}
```

**Example:**
```cpp
// Check for key press without blocking
if (Keyboard::IsKeyPressed()) {
    char key = Keyboard::ReadKey();

    // Check modifiers
    KeyboardStatus status = Keyboard::GetStatus();
    if (status.ctrlPressed && key == 'c') {
        // Ctrl+C pressed
    }
}

// Peek at key without consuming it
char nextKey = Keyboard::PeekKey();
```

---

# System.Windows.Forms Namespace

The System.Windows.Forms namespace contains classes for creating Windows-based applications with a rich user interface.

## Event Arguments

### PaintEventArgs

```cpp
namespace System::Windows::Forms {
    class PaintEventArgs {
    public:
        Graphics* graphics;   // Graphics context for drawing
        Rectangle bounds;     // Affected region
    };
}
```

### MouseEventArgs

```cpp
namespace System::Windows::Forms {
    class MouseEventArgs {
    public:
        int x, y;             // Screen coordinates
        bool leftButton;
        bool rightButton;
    };
}
```

### KeyboardEventArgs

```cpp
namespace System::Windows::Forms {
    class KeyboardEventArgs {
    public:
        char key;
        bool alt;
        bool ctrl;
        bool shift;
    };
}
```

---

## ControlType Enumeration

Used for runtime type identification of controls (similar to .NET's `GetType()`).

```cpp
namespace System::Windows::Forms {
    enum class ControlType {
        Control,    // Base control
        Desktop,    // Desktop surface
        Window,     // Top-level window
        TaskBar,    // Taskbar
        Button,     // Button control
        Picture     // Picture/image control
    };
}
```

---

## Layout System

WinDOS includes a CSS Flexbox-inspired layout system that automatically calculates and assigns control positions using a two-pass algorithm (Measure + Arrange). This eliminates the need for manual coordinate calculations and enables responsive layouts.

### Layout Enumerations

#### FlexDirection

Specifies the main axis direction for child layout.

```cpp
namespace System::Windows::Forms {
    enum class FlexDirection : unsigned char {
        Row,    // Children arranged horizontally (left to right)
        Column  // Children arranged vertically (top to bottom)
    };
}
```

#### JustifyContent

Specifies how children are distributed along the main axis.

```cpp
namespace System::Windows::Forms {
    enum class JustifyContent : unsigned char {
        Start,        // Pack children at the start
        Center,       // Center children
        End,          // Pack children at the end
        SpaceBetween, // Distribute with space between children
        SpaceAround   // Distribute with space around children
    };
}
```

#### AlignItems

Specifies how children are aligned on the cross axis.

```cpp
namespace System::Windows::Forms {
    enum class AlignItems : unsigned char {
        Start,   // Align to start of cross axis
        Center,  // Center on cross axis
        End,     // Align to end of cross axis
        Stretch  // Stretch to fill cross axis
    };
}
```

#### SizeMode

Specifies how a control's size is determined.

```cpp
namespace System::Windows::Forms {
    enum class SizeMode : unsigned char {
        Auto,  // Size based on content/children
        Fixed, // Use explicit bounds
        Fill   // Fill available space
    };
}
```

---

### LayoutProperties Structure

Embedded in each Control to configure layout behavior. Provides fluent setter methods for method chaining.

```cpp
namespace System::Windows::Forms {
    struct LayoutProperties {
        // Container properties (when control has children)
        FlexDirection direction = FlexDirection::Column;
        JustifyContent justifyContent = JustifyContent::Start;
        AlignItems alignItems = AlignItems::Stretch;
        Int32 gap = 0;  // Space between children in pixels

        // Self properties (when inside a flex container)
        Int32 flexGrow = 0;    // 0 = don't grow, 1+ = proportional growth
        Int32 flexShrink = 1;  // 0 = don't shrink (currently unused)

        // Sizing
        SizeMode widthMode = SizeMode::Auto;
        SizeMode heightMode = SizeMode::Auto;
        Int32 minWidth = 0;
        Int32 minHeight = 0;
        Int32 maxWidth = INT_MAX;
        Int32 maxHeight = INT_MAX;

        // Spacing (all sides)
        Int32 marginTop = 0, marginRight = 0, marginBottom = 0, marginLeft = 0;
        Int32 paddingTop = 0, paddingRight = 0, paddingBottom = 0, paddingLeft = 0;

        // Behavior
        bool participatesInLayout = true;  // false = floating (Windows)
        bool needsLayout = true;           // dirty flag

        // Fluent Setters (return *this for chaining)
        LayoutProperties& SetDirection(FlexDirection d);
        LayoutProperties& SetJustifyContent(JustifyContent jc);
        LayoutProperties& SetAlignItems(AlignItems ai);
        LayoutProperties& SetGap(Int32 g);
        LayoutProperties& SetFlexGrow(Int32 grow);
        LayoutProperties& SetFlexShrink(Int32 shrink);
        LayoutProperties& SetWidthMode(SizeMode mode);
        LayoutProperties& SetHeightMode(SizeMode mode);
        LayoutProperties& SetMinWidth(Int32 w);
        LayoutProperties& SetMinHeight(Int32 h);
        LayoutProperties& SetMaxWidth(Int32 w);
        LayoutProperties& SetMaxHeight(Int32 h);
        LayoutProperties& SetMargin(Int32 all);
        LayoutProperties& SetMargin(Int32 vertical, Int32 horizontal);
        LayoutProperties& SetMargin(Int32 top, Int32 right, Int32 bottom, Int32 left);
        LayoutProperties& SetPadding(Int32 all);
        LayoutProperties& SetPadding(Int32 vertical, Int32 horizontal);
        LayoutProperties& SetPadding(Int32 top, Int32 right, Int32 bottom, Int32 left);
        LayoutProperties& SetParticipatesInLayout(bool participates);
    };
}
```

---

### MeasureResult Structure

Returned by the Measure pass to report a control's preferred size.

```cpp
namespace System::Windows::Forms {
    struct MeasureResult {
        Int32 preferredWidth;
        Int32 preferredHeight;

        MeasureResult();
        MeasureResult(Int32 w, Int32 h);
    };
}
```

---

### Layout Algorithm

The layout system uses a two-pass algorithm:

**Phase 1: Measure (bottom-up)**
- Each control reports its preferred size based on content and children
- Children are measured first, then parent accumulates their sizes
- Respects min/max constraints

**Phase 2: Arrange (top-down)**
- Parent assigns final bounds to children based on:
  - Available space
  - FlexGrow distribution (extra space given to growing children)
  - JustifyContent positioning (main axis)
  - AlignItems alignment (cross axis)

---

### Layout Usage Example

```cpp
// Create a window with row layout for color spectrum controls
Window* window = new Window(&desktop, Rectangle(80, 60, 320, 240));

// Configure window as a flex container
window->Layout().SetDirection(FlexDirection::Row)
                .SetJustifyContent(JustifyContent::SpaceAround)
                .SetAlignItems(AlignItems::Stretch)
                .SetPadding(10);

// Create children with flexGrow to fill available space
SpectrumControl* red = new SpectrumControl(window,
    Rectangle(0, 0, 60, 100), Color::Red);
red->Layout().SetFlexGrow(1).SetMargin(5);

SpectrumControl* green = new SpectrumControl(window,
    Rectangle(0, 0, 60, 100), Color::Green);
green->Layout().SetFlexGrow(1).SetMargin(5);

SpectrumControl* blue = new SpectrumControl(window,
    Rectangle(0, 0, 60, 100), Color::Blue);
blue->Layout().SetFlexGrow(1).SetMargin(5);

// Trigger layout calculation
window->PerformLayout();

// Children are now automatically sized and positioned:
// - Each spectrum control grows to fill 1/3 of available width
// - All controls stretch to fill the window height
// - 5px margins separate the controls
// - SpaceAround adds equal space around each control
```

### Floating Controls (Windows)

Windows and other draggable controls should not participate in layout:

```cpp
// Windows are automatically floating (participatesInLayout = false)
Window* floatingWindow = new Window(&desktop, Rectangle(100, 100, 300, 200));

// Manually set for other controls if needed
myControl->Layout().SetParticipatesInLayout(false);
```

### TaskBar Layout

The TaskBar uses row layout internally:

```cpp
// TaskBar is configured as:
// - FlexDirection::Row
// - AlignItems::Center
// - Gap = 2
// - Start button (fixed), window buttons (flexGrow=1), system tray (fixed)
```

---

## Control Class

Defines the base class for controls, which are components with visual representation.

### Ownership Semantics

Parent controls **own** their children and are responsible for their lifetime. When a parent control is destroyed, it automatically deletes all its children. This follows the composite pattern used by Windows Forms and WPF.

**Rules:**
1. Children **must** be heap-allocated using `new`
2. Do **not** allocate children on the stack
3. Do **not** manually delete children that have a parent
4. `RemoveChild()` releases ownership - caller becomes responsible for deletion

**Correct Usage:**
```cpp
// Parent owns all children - automatic cleanup
Desktop desktop(Color::Cyan);
Window* win = new Window(&desktop, Rectangle(50, 50, 300, 200));
Button* btn = new Button(win, Rectangle(10, 10, 80, 25));
// No need to delete - Desktop destructor handles everything
```

**Incorrect Usage:**
```cpp
// WRONG - Stack allocation causes double-free crash!
Desktop desktop(Color::Cyan);
Window win(&desktop, Rectangle(50, 50, 300, 200));  // BAD!
// When desktop is destroyed, it tries to delete &win - CRASH!
```

**Transferring Ownership:**
```cpp
// Remove from one parent, add to another
parent1->RemoveChild(child);  // parent1 releases ownership
parent2->AddChild(child);     // parent2 takes ownership

// Remove and delete manually
parent->RemoveChild(child);   // parent releases ownership
delete child;                 // caller deletes
```

```cpp
namespace System::Windows::Forms {
    class Control {
    public:
        // Constructors
        Control();
        Control(Control* parent, const Rectangle& bounds);
        virtual ~Control();

        // Properties
        Control* Parent() const;
        const Rectangle& Bounds() const;          // Relative to parent
        const Rectangle& ClientBounds() const;    // Client area
        int ChildCount() const;
        Control* GetChild(int index) const;

        // Coordinate Methods
        Rectangle ScreenBounds() const;           // Absolute screen coords
        Rectangle ScreenClientBounds() const;
        Rectangle VisibleBounds() const;          // Clipped to parent

        // Bounds Manipulation
        void SetBounds(const Rectangle& bounds);
        void SetBounds(int x, int y, int width, int height);

        // Child Management (see Ownership Semantics above)
        void AddChild(Control* child);    // Takes ownership of child
        void RemoveChild(Control* child); // Releases ownership to caller

        // Painting (Override these)
        virtual void OnPaint(PaintEventArgs& e);
        virtual void OnPaintClient(PaintEventArgs& e);
        void Invalidate();                        // Mark for redraw
        void Update();                            // Redraw now

        // Input Events (Override these)
        virtual void OnMouse(MouseEventArgs& e);
        virtual void OnKeyboard(KeyboardEventArgs& e);

        // Event Propagation
        void NotifyMouse(MouseEventArgs& e);
        void NotifyKeyboard(KeyboardEventArgs& e);

        // Hit Testing
        bool HitTest(int x, int y) const;

        // Layout System
        LayoutProperties& Layout();                    // Get layout properties (mutable)
        const LayoutProperties& Layout() const;        // Get layout properties (const)
        virtual MeasureResult Measure(Int32 availableWidth, Int32 availableHeight);
        virtual void Arrange(const Rectangle& finalBounds);
        void PerformLayout();                          // Trigger full layout pass
        void InvalidateLayout();                       // Mark layout as dirty

        // Type Identification (similar to .NET GetType())
        virtual ControlType GetControlType() const;

        // Safe Type Casting (similar to .NET 'as' operator)
        // Returns nullptr if the control is not of the requested type
        virtual Window* AsWindow();
        virtual const Window* AsWindow() const;
        virtual Button* AsButton();
        virtual const Button* AsButton() const;
        virtual TaskBar* AsTaskBar();
        virtual const TaskBar* AsTaskBar() const;
        virtual Picture* AsPicture();
        virtual const Picture* AsPicture() const;

        // Type Checking (similar to .NET 'is' operator)
        Boolean IsWindow() const;
        Boolean IsButton() const;
        Boolean IsTaskBar() const;
        Boolean IsPicture() const;
        Boolean IsDesktop() const;
    };
}
```

**Type Identification Example:**
```cpp
// Safely cast a control to a Window
Control* ctrl = GetControlAtPosition(x, y);

// Using 'as' pattern - returns nullptr if not a Window
Window* win = ctrl->AsWindow();
if (win) {
    win->SetFocused(true);
}

// Using 'is' pattern - check type first
if (ctrl->IsButton()) {
    Button* btn = ctrl->AsButton();
    // Handle button click
}

// Using GetControlType() for switch statements
switch (ctrl->GetControlType()) {
    case ControlType::Window:
        // Handle window
        break;
    case ControlType::Button:
        // Handle button
        break;
    default:
        break;
}
```

**Creating a Custom Control:**
```cpp
class MyControl : public Control {
public:
    MyControl(Control* parent, const Rectangle& bounds)
        : Control(parent, bounds) {}

    void OnPaint(PaintEventArgs& e) override {
        // Draw control background
        e.graphics->FillRectangle(Bounds(), Color::Gray);

        // Call base to draw children
        Control::OnPaint(e);
    }

    void OnPaintClient(PaintEventArgs& e) override {
        // Draw client area content
        e.graphics->DrawLine(0, 0,
            ClientBounds().width, ClientBounds().height,
            Color::White);
    }

    void OnMouse(MouseEventArgs& e) override {
        if (e.leftButton) {
            // Handle click
        }
        Control::OnMouse(e);
    }
};
```

---

## Desktop Class

Represents the desktop surface that hosts all windows and controls. The Desktop automatically adapts to the current display mode's resolution. Supports desktop icons and custom mouse cursors.

```cpp
namespace System::Windows::Forms {
    // Desktop icon data structure
    struct DesktopIcon {
        Image image;    // 32x32 icon image
        Int32 x, y;     // Position on desktop
    };

    class Desktop : public Control {
    public:
        // Constructors
        Desktop(const Color& backgroundColor = Color::Cyan);
        virtual ~Desktop();

        // Window Focus
        Window* FocusedWindow() const;
        void SetFocusedWindow(Window* window);

        // Screen Properties
        Int32 ScreenWidth() const;        // Current display width
        Int32 ScreenHeight() const;       // Current display height

        // Desktop Icons
        void AddIcon(const Image& icon);  // Add icon with auto-positioning
        void AddIconFromLibrary(const char* path, Int32 iconIndex);
        Int32 IconCount() const;

        // Custom Cursor
        void SetCursor(const Image& cursorImage);
        void LoadCursorFromLibrary(const char* path, Int32 iconIndex);

        // Event Loop
        void Run();                       // Start event loop
        void Stop();                      // Exit event loop

        // Overrides
        virtual void OnPaint(PaintEventArgs& e) override;
        virtual void OnKeyboard(KeyboardEventArgs& e) override;
        void HandleMouse(MouseEventArgs& e);
    };
}
```

**Example:**
```cpp
// Create desktop (automatically adapts to current display mode)
Desktop desktop(Color::Cyan);

// Query screen dimensions
Int32 screenW = desktop.ScreenWidth();   // e.g., 640 for VGA_640x480x4
Int32 screenH = desktop.ScreenHeight();  // e.g., 480 for VGA_640x480x4

// Add desktop icons (auto-positioned in a grid)
desktop.AddIconFromLibrary("shell32.dll", 3);   // Folder icon
desktop.AddIconFromLibrary("shell32.dll", 15);  // Computer icon
desktop.AddIconFromLibrary("shell32.dll", 32);  // Recycle bin

// Or add custom icon images
Image myIcon = Image::FromIcon("myapp.ico", Size::IconMedium);
desktop.AddIcon(myIcon);

// Set custom mouse cursor (24x24 recommended)
Image cursorImage = Image::FromIcon("cursor.ico", Size::IconCursor);
desktop.SetCursor(cursorImage);

// Or load cursor from icon library
desktop.LoadCursorFromLibrary("cursors.dll", 0);

// Add windows
Window* mainWindow = new Window(&desktop, Rectangle(50, 50, 400, 300));
Window* toolWindow = new Window(&desktop, Rectangle(200, 100, 200, 150));

// Start the application
desktop.Run();  // Blocks until ESC is pressed
```

---

## Window Class

Represents a window with title bar and frame.

```cpp
namespace System::Windows::Forms {
    class Window : public Control {
    public:
        // Constants
        static const int TITLE_BAR_HEIGHT = 20;
        static const int FRAME_WIDTH = 3;

        // Constructors
        Window(Control* parent, const Rectangle& bounds);
        virtual ~Window();

        // Properties
        bool IsFocused() const;
        void SetFocused(bool focused);

        // Overrides
        virtual void UpdateClientBounds() override;
        virtual void OnPaint(PaintEventArgs& e) override;
        virtual void OnMouse(MouseEventArgs& e) override;
    };
}
```

**Example:**
```cpp
// Create a window on the desktop
Window* window = new Window(&desktop, Rectangle(100, 100, 300, 200));

// The window automatically:
// - Draws a title bar (20px high)
// - Draws a 3D frame border
// - Provides a client area for child controls
// - Handles focus changes
// - Supports dragging via title bar
```

---

## TaskBar Class

Represents the Windows 95-style taskbar with Start button and window list.

```cpp
namespace System::Windows::Forms {
    class TaskBar : public Control {
    public:
        // Constructors
        TaskBar(Control* parent, StartMenu* startMenu = nullptr);
        virtual ~TaskBar();

        // Properties
        Button* StartButton() const;

        // Desktop back-reference (set by Desktop::SetTaskBar)
        void SetDesktop(Desktop* desktop);
        Desktop* GetDesktop() const;

        // Window button management
        void AddWindowButton(Window* window);
        void RemoveWindowButton(Window* window);
        void RefreshWindowButtons();  // Update pressed states based on focus
        TaskBarButton* FindButtonForWindow(Window* window) const;

        // Overrides
        virtual void OnPaint(PaintEventArgs& e) override;
    };
}
```

**Window List Behavior:**
- Window buttons are automatically created when windows are added to the Desktop
- Clicking a window button focuses the corresponding window
- The focused window's button appears pressed/sunken
- Window buttons are automatically removed when windows are removed

**Example:**
```cpp
Desktop desktop(Color::Cyan);

// Create start menu and taskbar
StartMenu* startMenu = new StartMenu(&desktop);
desktop.SetStartMenu(startMenu);

TaskBar* taskbar = new TaskBar(&desktop, startMenu);
desktop.SetTaskBar(taskbar);
taskbar->SetDesktop(&desktop);

// Create windows - taskbar buttons are created automatically
Window* window1 = new Window(&desktop, Rectangle(50, 50, 300, 200));
Window* window2 = new Window(&desktop, Rectangle(200, 100, 300, 200));

// TaskBar now shows:
// - Start button (opens/closes start menu)
// - Window1 button (click to focus window1)
// - Window2 button (click to focus window2)
// - Focused window's button appears pressed
```

---

## TaskBarButton Class

A specialized button that represents an open window in the taskbar.

```cpp
namespace System::Windows::Forms {
    class TaskBarButton : public Button {
    public:
        // Constructors
        TaskBarButton(Control* parent, const Rectangle& bounds, Window* window);
        virtual ~TaskBarButton();

        // Properties
        Window* GetWindow() const;  // The window this button represents
    };
}
```

**Behavior:**
- Automatically created by TaskBar when windows are added to Desktop
- Shows pressed/sunken when the associated window is focused
- Clicking the button focuses the associated window
- Inherits toggle state behavior from Button class

---

## StartMenu Class

Represents the Windows 95-style start menu that appears when clicking the Start button.

```cpp
namespace System::Windows::Forms {
    class StartMenu : public Control {
    public:
        // Constructors
        StartMenu(Control* parent);
        virtual ~StartMenu();

        // Visibility
        void Show();
        void Hide();
        Boolean IsVisible() const;

        // Overrides
        virtual void OnPaint(PaintEventArgs& e) override;
        virtual void OnMouse(MouseEventArgs& e) override;
    };
}
```

**Example:**
```cpp
Desktop desktop(Color::Cyan);

// Create start menu (typically created automatically by TaskBar)
StartMenu* startMenu = new StartMenu(&desktop);
desktop.SetStartMenu(startMenu);

// Create taskbar with start menu reference
TaskBar* taskbar = new TaskBar(&desktop, startMenu);
desktop.SetTaskBar(taskbar);

// Start menu automatically:
// - Uses column layout for menu items
// - Includes a branded sidebar on the left
// - Shows/hides when Start button is clicked
// - Menu items highlight on hover (blue background)
```

---

## MenuItem Class

Represents a single item in a menu. Supports hover highlighting.

```cpp
namespace System::Windows::Forms {
    class MenuItem : public Control {
    public:
        // Constructors
        MenuItem(Control* parent, const Rectangle& bounds);
        virtual ~MenuItem();

        // Highlight State
        Boolean IsHighlighted() const;
        void SetHighlighted(Boolean highlighted);

        // Overrides
        virtual void OnPaint(PaintEventArgs& e) override;
        virtual void OnMouse(MouseEventArgs& e) override;
    };
}
```

**Example:**
```cpp
// Menu items are typically created by StartMenu internally
// They automatically:
// - Show blue background on mouse hover
// - Revert to gray background on mouse leave
// - Handle click events
```

---

## Button Class

Represents a Windows button control. Supports both temporary press (during mouse click) and persistent toggle state (for taskbar buttons, menu buttons).

```cpp
namespace System::Windows::Forms {
    class Button : public Control {
    public:
        // Constructors
        Button(Control* parent, const Rectangle& bounds);
        virtual ~Button();

        // Properties
        Boolean IsPressed() const;  // True if toggled OR mouse down

        // Set persistent pressed/toggle state
        // Used by TaskBar buttons to show active window
        void SetPressed(Boolean pressed);

        // Click event handler
        void SetOnClick(ClickEventHandler handler, void* userData = nullptr);

        // Overrides
        virtual void OnPaint(PaintEventArgs& e) override;
        virtual void OnMouse(MouseEventArgs& e) override;
    };
}
```

**Button States:**
- **Toggle state**: Persistent pressed appearance (set via `SetPressed()`)
- **Mouse state**: Temporary pressed appearance during click
- **Visual state**: Button appears pressed if either toggle OR mouse state is active

**Example:**
```cpp
Window* window = new Window(&desktop, Rectangle(100, 100, 300, 200));

// Create a button in the window's client area
Button* okButton = new Button(window, Rectangle(10, 10, 80, 25));

// Set click handler
okButton->SetOnClick([](Button* btn, void* data) {
    // Handle click
}, nullptr);

// Button automatically:
// - Draws with 3D raised appearance
// - Shows pressed state on click
// - Handles mouse input

// For toggle buttons (like Start button when menu is open):
startButton->SetPressed(true);   // Shows as pressed
startButton->SetPressed(false);  // Shows as normal
```

---

## Picture Class

Displays an image.

```cpp
namespace System::Windows::Forms {
    class Picture : public Control {
    public:
        // Constructors
        Picture(Control* parent, const Rectangle& bounds);
        Picture(Control* parent, const Rectangle& bounds, const Image& image);
        virtual ~Picture();

        // Methods
        void SetImage(const Image& image);
        const Image& GetImage() const;

        // Overrides
        virtual void OnPaint(PaintEventArgs& e) override;
    };
}
```

**Example:**
```cpp
// Load an image
Image photo = Image::FromBitmap("assets\\photo.bmp");

// Create picture control
Window* window = new Window(&desktop, Rectangle(50, 50, 200, 200));
Picture* picture = new Picture(window,
    Rectangle(0, 0,
              window->ClientBounds().width,
              window->ClientBounds().height),
    photo);
```

---

## SpectrumControl Class

Displays a vertical color gradient for color selection (white → base color → black).

```cpp
namespace System::Windows::Forms {
    class SpectrumControl : public Control {
    public:
        // Constructors
        SpectrumControl(Control* parent, const Rectangle& bounds);
        SpectrumControl(Control* parent, const Rectangle& bounds,
                        const Color& baseColor);
        virtual ~SpectrumControl();

        // Properties
        Color GetBaseColor() const;
        void SetBaseColor(const Color& color);

        // Get color at specific Y position within control
        Color GetColorAtY(Int32 y) const;

        // Overrides
        virtual void OnPaint(PaintEventArgs& e) override;
    };
}
```

**Example:**
```cpp
// Create a color picker window
Window* colorPicker = new Window(&desktop, Rectangle(100, 100, 200, 300));

// Add spectrum control showing red gradient
SpectrumControl* spectrum = new SpectrumControl(colorPicker,
    Rectangle(10, 10, 30, 200), Color::Red);

// Get color at click position
void OnSpectrumClick(Int32 y) {
    Color selectedColor = spectrum->GetColorAtY(y);
    // Use selected color...
}

// Change base color
spectrum->SetBaseColor(Color::Blue);  // Now shows white → blue → black
```

---

# Platform.DOS Namespace

The Platform.DOS namespace contains low-level DOS-specific implementations. These are internal APIs used by the higher-level System classes.

## DOSSystem Class

Basic DOS system operations.

```cpp
namespace Platform::DOS {
    class DOSSystem {
    public:
        static void WriteString(const char* s);
        static void WriteChar(char c);
        static char ReadChar();
        static int ReadLine(char* buffer, int maxLength);
        static void Exit(int code);
        static void GetVersion(int& major, int& minor);
    };
}
```

## Video Class

Text-mode video operations.

```cpp
namespace Platform::DOS {
    class Video {
    public:
        static void SetCursorPosition(int row, int col);
        static void GetCursorPosition(int& row, int& col);
        static void WriteChar(char c, unsigned char attr);
        static void WriteCharAtCursor(char c);
        static void SetVideoMode(int mode);
        static void ScrollUp(int lines, unsigned char attr,
                            int top, int left, int bottom, int right);
        static void GetScreenSize(int& rows, int& cols);
        static void ClearScreen(unsigned char attr);
    };
}
```

## Graphics Class

VGA graphics operations.

```cpp
namespace Platform::DOS {
    class Graphics {
    public:
        static void SetVideoMode(unsigned char mode);
        static unsigned char GetVideoMode();
        static void WaitForVSync();
        static void SelectPlane(int plane);
        static void CopyToVGA(const void* data, unsigned int offset,
                             unsigned int length);
        static void OutPort(unsigned short port, unsigned char value);
        static unsigned char InPort(unsigned short port);
    };
}
```

## Keyboard Class

Raw keyboard access.

```cpp
namespace Platform::DOS {
    class Keyboard {
    public:
        static char ReadChar();
        static bool IsKeyAvailable();
        static unsigned short ReadScanCode();
        static unsigned short PeekKey();
    };
}
```

## Mouse Class

Raw mouse driver access.

```cpp
namespace Platform::DOS {
    struct MouseState {
        int x, y;
        bool leftButton, rightButton, middleButton;
    };

    class Mouse {
    public:
        static bool Initialize();
        static void ShowCursor();
        static void HideCursor();
        static MouseState GetState();
        static void SetPosition(int x, int y);
        static void SetHorizontalBounds(int min, int max);
        static void SetVerticalBounds(int min, int max);
    };
}
```

---

# Complete Example: Windows 95-Style Application

```cpp
// main.cpp - Complete WinDOS Application
#include "bcl/BCL.hpp"

using namespace System;
using namespace System::Drawing;
using namespace System::Devices;
using namespace System::Windows::Forms;

int main() {
    // Initialize hardware
    Mouse::Initialize();
    Display::SetMode(Display::VGA_640x480x4);
    GraphicsBuffer::CreateFrameBuffer(640, 480, 0x12);
    Mouse::HideCursor();
    Mouse::SetBounds(0, 0, 639, 479);

    // Create desktop environment
    Desktop desktop(Color::Cyan);

    // Create taskbar
    TaskBar* taskbar = new TaskBar(&desktop);

    // Create main window
    Window* mainWindow = new Window(&desktop, Rectangle(50, 50, 400, 300));

    // Create tool window
    Window* toolWindow = new Window(&desktop, Rectangle(300, 150, 200, 150));

    // Load and display an image
    Image logo = Image::FromBitmap("assets\\logo.bmp");
    Picture* logoPic = new Picture(mainWindow,
        Rectangle(10, 10, logo.Width(), logo.Height()),
        logo);

    // Create a button
    Button* okButton = new Button(mainWindow,
        Rectangle(10, mainWindow->ClientBounds().height - 35, 80, 25));

    // Suppress unused variable warnings
    (void)taskbar;
    (void)toolWindow;
    (void)logoPic;
    (void)okButton;

    // Run the application (ESC to exit)
    desktop.Run();

    // Cleanup
    GraphicsBuffer::DestroyFrameBuffer();
    Display::SetDefaultMode();

    return 0;
}
```

**Build and Run:**
```bash
make forms_demo
# Copy forms.exe and CWSDPMI.EXE to DOS environment
# Run: forms.exe
```

---

# Appendix

## VGA Video Modes

| Mode | Resolution | Colors | Type |
|------|------------|--------|------|
| 0x03 | 80x25 | 16 | Text |
| 0x12 | 640x480 | 16 | Graphics (Planar) |
| 0x13 | 320x200 | 256 | Graphics (Linear) |

## VGA 16-Color Palette

| Index | Color | RGB |
|-------|-------|-----|
| 0 | Black | (0, 0, 0) |
| 1 | Dark Blue | (0, 0, 170) |
| 2 | Dark Green | (0, 170, 0) |
| 3 | Dark Cyan | (0, 170, 170) |
| 4 | Dark Red | (170, 0, 0) |
| 5 | Dark Magenta | (170, 0, 170) |
| 6 | Dark Yellow (Brown) | (170, 85, 0) |
| 7 | Gray | (170, 170, 170) |
| 8 | Dark Gray | (85, 85, 85) |
| 9 | Blue | (85, 85, 255) |
| 10 | Green | (85, 255, 85) |
| 11 | Cyan | (85, 255, 255) |
| 12 | Red | (255, 85, 85) |
| 13 | Magenta | (255, 85, 255) |
| 14 | Yellow | (255, 255, 85) |
| 15 | White | (255, 255, 255) |

## Keyboard Scan Codes (Common)

| Key | Scan Code |
|-----|-----------|
| ESC | 0x01 |
| Enter | 0x1C |
| Space | 0x39 |
| Arrow Up | 0x48 |
| Arrow Down | 0x50 |
| Arrow Left | 0x4B |
| Arrow Right | 0x4D |

## BMP File Format (4-bit)

WinDOS supports loading 4-bit (16-color) Windows BMP files:
- File header: 14 bytes
- Info header: 40 bytes (BITMAPINFOHEADER)
- Palette: 16 entries x 4 bytes (BGRA)
- Pixel data: Bottom-up, 4-bit packed (2 pixels per byte)

---

*WinDOS - Bringing Windows 95 to DOS, one pixel at a time.*
