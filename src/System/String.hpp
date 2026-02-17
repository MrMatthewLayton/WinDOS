#ifndef SYSTEM_STRING_HPP
#define SYSTEM_STRING_HPP

// Forward declare the types we need (they're defined in Types.hpp)
// This avoids circular include issues while allowing String.hpp to be included standalone
namespace System {
    class Int32;
    class Boolean;
    class Char;
}

namespace System {

// Forward declarations
template<typename T> class Array;

class String {
private:
    char* _data;
    int _length;  // Internal storage stays as primitive for efficiency

    void _copy(const char* src, int len);
    void _free();

public:
    // Constructors
    String();
    String(const char* s);
    String(const char* s, Int32 length);
    String(Char c, Int32 count);
    String(const String& other);
    String(String&& other) noexcept;
    ~String();

    // Assignment
    String& operator=(const String& other);
    String& operator=(String&& other) noexcept;
    String& operator=(const char* s);

    // Properties
    Int32 Length() const;
    Boolean IsEmpty() const;
    Char operator[](Int32 index) const;
    const char* CStr() const;

    // Substring
    String Substring(Int32 startIndex) const;
    String Substring(Int32 startIndex, Int32 length) const;

    // Search
    Int32 IndexOf(Char c) const;
    Int32 IndexOf(Char c, Int32 startIndex) const;
    Int32 IndexOf(const String& s) const;
    Int32 IndexOf(const String& s, Int32 startIndex) const;
    Int32 LastIndexOf(Char c) const;
    Int32 LastIndexOf(const String& s) const;
    Boolean Contains(const String& s) const;
    Boolean StartsWith(const String& s) const;
    Boolean EndsWith(const String& s) const;

    // Transformation
    String ToUpper() const;
    String ToLower() const;
    String Trim() const;
    String TrimStart() const;
    String TrimEnd() const;
    String Replace(Char oldChar, Char newChar) const;
    String Replace(const String& oldValue, const String& newValue) const;
    String Insert(Int32 startIndex, const String& value) const;
    String Remove(Int32 startIndex) const;
    String Remove(Int32 startIndex, Int32 count) const;
    String PadLeft(Int32 totalWidth, char paddingChar = ' ') const;
    String PadRight(Int32 totalWidth, char paddingChar = ' ') const;

    // Split
    Array<String> Split(Char delimiter) const;
    Array<String> Split(const char* delimiters) const;

    // Comparison operators
    bool operator==(const String& other) const;
    bool operator!=(const String& other) const;
    bool operator<(const String& other) const;
    bool operator>(const String& other) const;
    bool operator<=(const String& other) const;
    bool operator>=(const String& other) const;
    bool operator==(const char* other) const;
    bool operator!=(const char* other) const;

    // Concatenation
    String operator+(const String& other) const;
    String operator+(const char* other) const;
    String operator+(Char c) const;
    String& operator+=(const String& other);
    String& operator+=(const char* other);
    String& operator+=(Char c);

    // Static members
    static const String Empty;
    static Boolean IsNullOrEmpty(const String& s);
    static Boolean IsNullOrWhiteSpace(const String& s);
    static String Concat(const String& s1, const String& s2);
    static String Concat(const String& s1, const String& s2, const String& s3);
    static Int32 Compare(const String& s1, const String& s2);
    static Int32 CompareIgnoreCase(const String& s1, const String& s2);

    // Comparison
    Int32 CompareTo(const String& other) const;
    Boolean Equals(const String& other) const;
    Boolean EqualsIgnoreCase(const String& other) const;

    // Hashing (simple)
    Int32 GetHashCode() const;
};

// Non-member operators
String operator+(const char* lhs, const String& rhs);

/******************************************************************************/
/*    System::StringBuilder                                                    */
/******************************************************************************/

/**
 * Represents a mutable string of characters with efficient append operations.
 *
 * Use StringBuilder when concatenating many strings in a loop or building
 * strings incrementally. Unlike String (which is immutable), StringBuilder
 * modifies its internal buffer in-place, avoiding allocations.
 *
 * Example:
 *   StringBuilder sb;
 *   for (int i = 0; i < 100; i++) {
 *       sb.Append("Line ").Append(i).AppendLine();
 *   }
 *   String result = sb.ToString();
 */
class StringBuilder {
private:
    char* _buffer;
    int _length;      // Current string length
    int _capacity;    // Buffer capacity (always > _length for null terminator)

    static const int DEFAULT_CAPACITY = 16;
    static const int GROWTH_FACTOR = 2;

    void EnsureCapacity(int minCapacity);

public:
    // Constructors
    StringBuilder();
    explicit StringBuilder(Int32 capacity);
    explicit StringBuilder(const String& value);
    StringBuilder(const StringBuilder& other);
    StringBuilder(StringBuilder&& other) noexcept;
    ~StringBuilder();

    // Assignment
    StringBuilder& operator=(const StringBuilder& other);
    StringBuilder& operator=(StringBuilder&& other) noexcept;

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

    // Insert (at position)
    StringBuilder& Insert(Int32 index, const String& value);
    StringBuilder& Insert(Int32 index, const char* value);
    StringBuilder& Insert(Int32 index, Char value);

    // Remove
    StringBuilder& Remove(Int32 startIndex, Int32 length);

    // Clear
    StringBuilder& Clear();

    // Reserve capacity
    void Reserve(Int32 capacity);

    // Convert to String
    String ToString() const;
};

} // namespace System

#endif // SYSTEM_STRING_HPP
