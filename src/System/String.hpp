#ifndef SYSTEM_STRING_HPP
#define SYSTEM_STRING_HPP

// Forward declare the types we need (they're defined in Types.hpp)
// This avoids circular include issues while allowing String.hpp to be included standalone
namespace System
{
    class Int32;
    class Boolean;
    class Char;
}

namespace System
{

// Forward declarations
template<typename T> class Array;

/// @brief Represents an immutable sequence of Unicode characters.
///
/// The String class provides a rich set of methods for string manipulation,
/// searching, and comparison. Strings in this implementation are immutable -
/// all transformation methods return new String instances rather than modifying
/// the original.
///
/// For efficient string building in loops, use StringBuilder instead.
///
/// @note This class mirrors the .NET System.String API.
class String
{
private:
    char* _data;
    Int32 _length;

    void _copy(const char* src, Int32 len);
    void _free();

public:
    // Constructors

    /// @brief Initializes a new instance of the String class to an empty string.
    String();

    /// @brief Initializes a new instance of the String class from a null-terminated C string.
    /// @param s A pointer to a null-terminated array of characters. If null, creates an empty string.
    String(const char* s);

    /// @brief Initializes a new instance of the String class from a character array with specified length.
    /// @param s A pointer to an array of characters.
    /// @param length The number of characters to copy from the array.
    /// @throws ArgumentOutOfRangeException If length is negative.
    String(const char* s, Int32 length);

    /// @brief Initializes a new instance of the String class to a specified character repeated a specified number of times.
    /// @param c The character to repeat.
    /// @param count The number of times to repeat the character.
    /// @throws ArgumentOutOfRangeException If count is negative.
    String(Char c, Int32 count);

    /// @brief Initializes a new instance of the String class as a copy of another String.
    /// @param other The String to copy.
    String(const String& other);

    /// @brief Initializes a new instance of the String class by moving from another String.
    /// @param other The String to move from. Will be left in an empty state.
    String(String&& other) noexcept;

    /// @brief Destroys the String instance and releases its memory.
    ~String();

    // Assignment

    /// @brief Assigns the value of another String to this instance.
    /// @param other The String to copy from.
    /// @return A reference to this String.
    String& operator=(const String& other);

    /// @brief Moves the value of another String to this instance.
    /// @param other The String to move from. Will be left in an empty state.
    /// @return A reference to this String.
    String& operator=(String&& other) noexcept;

    /// @brief Assigns a null-terminated C string to this instance.
    /// @param s A pointer to a null-terminated array of characters.
    /// @return A reference to this String.
    String& operator=(const char* s);

    // Properties

    /// @brief Gets the number of characters in this String.
    /// @return The length of the string.
    Int32 Length() const;

    /// @brief Gets a value indicating whether this String has zero length.
    /// @return True if the string length is zero; otherwise, false.
    Boolean IsEmpty() const;

    /// @brief Gets the character at the specified index.
    /// @param index The zero-based index of the character.
    /// @return The character at the specified position.
    /// @throws IndexOutOfRangeException If index is less than zero or greater than or equal to Length().
    Char operator[](Int32 index) const;

    /// @brief Gets a pointer to the underlying null-terminated character array.
    /// @return A pointer to the null-terminated C string representation.
    const char* CStr() const;

    // Substring

    /// @brief Retrieves a substring from this instance starting at a specified position.
    /// @param startIndex The zero-based starting character position.
    /// @return A String that is equivalent to the substring that begins at startIndex, or Empty if startIndex equals Length().
    /// @throws ArgumentOutOfRangeException If startIndex is less than zero or greater than Length().
    String Substring(Int32 startIndex) const;

    /// @brief Retrieves a substring from this instance with specified starting position and length.
    /// @param startIndex The zero-based starting character position.
    /// @param length The number of characters in the substring.
    /// @return A String of length characters beginning at startIndex.
    /// @throws ArgumentOutOfRangeException If startIndex or length is negative, or startIndex + length exceeds Length().
    String Substring(Int32 startIndex, Int32 length) const;

    // Search

    /// @brief Reports the zero-based index of the first occurrence of the specified character.
    /// @param c The character to seek.
    /// @return The index of the character if found; otherwise, -1.
    Int32 IndexOf(Char c) const;

    /// @brief Reports the zero-based index of the first occurrence of the specified character, starting at a specified position.
    /// @param c The character to seek.
    /// @param startIndex The search starting position.
    /// @return The index of the character if found; otherwise, -1.
    /// @throws ArgumentOutOfRangeException If startIndex is negative or greater than Length().
    Int32 IndexOf(Char c, Int32 startIndex) const;

    /// @brief Reports the zero-based index of the first occurrence of the specified string.
    /// @param s The string to seek.
    /// @return The index of the string if found; otherwise, -1.
    Int32 IndexOf(const String& s) const;

    /// @brief Reports the zero-based index of the first occurrence of the specified string, starting at a specified position.
    /// @param s The string to seek.
    /// @param startIndex The search starting position.
    /// @return The index of the string if found; otherwise, -1.
    /// @throws ArgumentOutOfRangeException If startIndex is negative or greater than Length().
    Int32 IndexOf(const String& s, Int32 startIndex) const;

    /// @brief Reports the zero-based index of the last occurrence of the specified character.
    /// @param c The character to seek.
    /// @return The index of the character if found; otherwise, -1.
    Int32 LastIndexOf(Char c) const;

    /// @brief Reports the zero-based index of the last occurrence of the specified string.
    /// @param s The string to seek.
    /// @return The index of the string if found; otherwise, -1.
    Int32 LastIndexOf(const String& s) const;

    /// @brief Determines whether this string contains the specified substring.
    /// @param s The string to seek.
    /// @return True if the substring occurs within this string; otherwise, false.
    Boolean Contains(const String& s) const;

    /// @brief Determines whether the beginning of this string matches the specified string.
    /// @param s The string to compare.
    /// @return True if this string begins with s; otherwise, false.
    Boolean StartsWith(const String& s) const;

    /// @brief Determines whether the end of this string matches the specified string.
    /// @param s The string to compare.
    /// @return True if this string ends with s; otherwise, false.
    Boolean EndsWith(const String& s) const;

    // Transformation

    /// @brief Returns a copy of this string converted to uppercase.
    /// @return A new String in uppercase.
    String ToUpper() const;

    /// @brief Returns a copy of this string converted to lowercase.
    /// @return A new String in lowercase.
    String ToLower() const;

    /// @brief Removes all leading and trailing whitespace characters from this string.
    /// @return A new String with whitespace removed from both ends.
    String Trim() const;

    /// @brief Removes all leading whitespace characters from this string.
    /// @return A new String with whitespace removed from the beginning.
    String TrimStart() const;

    /// @brief Removes all trailing whitespace characters from this string.
    /// @return A new String with whitespace removed from the end.
    String TrimEnd() const;

    /// @brief Returns a new string in which all occurrences of a specified character are replaced with another character.
    /// @param oldChar The character to be replaced.
    /// @param newChar The character to replace all occurrences of oldChar.
    /// @return A new String with the replacements made.
    String Replace(Char oldChar, Char newChar) const;

    /// @brief Returns a new string in which all occurrences of a specified string are replaced with another string.
    /// @param oldValue The string to be replaced.
    /// @param newValue The string to replace all occurrences of oldValue.
    /// @return A new String with the replacements made.
    /// @throws ArgumentException If oldValue is empty.
    /// @throws OverflowException If the result would exceed maximum string length.
    String Replace(const String& oldValue, const String& newValue) const;

    /// @brief Returns a new string in which a specified string is inserted at a specified index position.
    /// @param startIndex The zero-based index position of the insertion.
    /// @param value The string to insert.
    /// @return A new String with the value inserted.
    /// @throws ArgumentOutOfRangeException If startIndex is negative or greater than Length().
    String Insert(Int32 startIndex, const String& value) const;

    /// @brief Returns a new string in which all characters from a specified position to the end are deleted.
    /// @param startIndex The zero-based position to begin deleting characters.
    /// @return A new String equivalent to this string without the removed characters.
    /// @throws ArgumentOutOfRangeException If startIndex is negative or greater than Length().
    String Remove(Int32 startIndex) const;

    /// @brief Returns a new string in which a specified number of characters are deleted beginning at a specified position.
    /// @param startIndex The zero-based position to begin deleting characters.
    /// @param count The number of characters to delete.
    /// @return A new String equivalent to this string without the removed characters.
    /// @throws ArgumentOutOfRangeException If startIndex or count is negative, or startIndex + count exceeds Length().
    String Remove(Int32 startIndex, Int32 count) const;

    /// @brief Returns a new string that right-aligns the characters by padding with spaces on the left.
    /// @param totalWidth The total number of characters in the resulting string.
    /// @return A new String of length totalWidth, or the original string if totalWidth is less than Length().
    String PadLeft(Int32 totalWidth) const;

    /// @brief Returns a new string that right-aligns the characters by padding with a specified character on the left.
    /// @param totalWidth The total number of characters in the resulting string.
    /// @param paddingChar The padding character.
    /// @return A new String of length totalWidth, or the original string if totalWidth is less than Length().
    String PadLeft(Int32 totalWidth, Char paddingChar) const;

    /// @brief Returns a new string that left-aligns the characters by padding with spaces on the right.
    /// @param totalWidth The total number of characters in the resulting string.
    /// @return A new String of length totalWidth, or the original string if totalWidth is less than Length().
    String PadRight(Int32 totalWidth) const;

    /// @brief Returns a new string that left-aligns the characters by padding with a specified character on the right.
    /// @param totalWidth The total number of characters in the resulting string.
    /// @param paddingChar The padding character.
    /// @return A new String of length totalWidth, or the original string if totalWidth is less than Length().
    String PadRight(Int32 totalWidth, Char paddingChar) const;

    // Split

    /// @brief Splits this string into an array of substrings based on a delimiter character.
    /// @param delimiter The character that delimits the substrings.
    /// @return An Array containing the substrings.
    Array<String> Split(Char delimiter) const;

    /// @brief Splits this string into an array of substrings based on multiple delimiter characters.
    /// @param delimiters A null-terminated string containing delimiter characters.
    /// @return An Array containing the substrings.
    Array<String> Split(const char* delimiters) const;

    // Comparison operators

    /// @brief Determines whether this string and another string have the same value.
    /// @param other The String to compare to this instance.
    /// @return True if the strings are equal; otherwise, false.
    Boolean operator==(const String& other) const;

    /// @brief Determines whether this string and another string have different values.
    /// @param other The String to compare to this instance.
    /// @return True if the strings are not equal; otherwise, false.
    Boolean operator!=(const String& other) const;

    /// @brief Determines whether this string precedes another string in sort order.
    /// @param other The String to compare to this instance.
    /// @return True if this string precedes other; otherwise, false.
    Boolean operator<(const String& other) const;

    /// @brief Determines whether this string follows another string in sort order.
    /// @param other The String to compare to this instance.
    /// @return True if this string follows other; otherwise, false.
    Boolean operator>(const String& other) const;

    /// @brief Determines whether this string precedes or equals another string in sort order.
    /// @param other The String to compare to this instance.
    /// @return True if this string precedes or equals other; otherwise, false.
    Boolean operator<=(const String& other) const;

    /// @brief Determines whether this string follows or equals another string in sort order.
    /// @param other The String to compare to this instance.
    /// @return True if this string follows or equals other; otherwise, false.
    Boolean operator>=(const String& other) const;

    /// @brief Determines whether this string and a C string have the same value.
    /// @param other The C string to compare to this instance.
    /// @return True if the strings are equal; otherwise, false.
    Boolean operator==(const char* other) const;

    /// @brief Determines whether this string and a C string have different values.
    /// @param other The C string to compare to this instance.
    /// @return True if the strings are not equal; otherwise, false.
    Boolean operator!=(const char* other) const;

    // Concatenation

    /// @brief Concatenates this string with another string.
    /// @param other The String to concatenate.
    /// @return A new String that is the concatenation of this string and other.
    /// @throws OverflowException If the result would exceed maximum string length.
    String operator+(const String& other) const;

    /// @brief Concatenates this string with a C string.
    /// @param other The C string to concatenate.
    /// @return A new String that is the concatenation of this string and other.
    /// @throws OverflowException If the result would exceed maximum string length.
    String operator+(const char* other) const;

    /// @brief Concatenates this string with a character.
    /// @param c The character to concatenate.
    /// @return A new String that is this string followed by the character.
    String operator+(Char c) const;

    /// @brief Appends another string to this string.
    /// @param other The String to append.
    /// @return A reference to this String.
    /// @throws OverflowException If the result would exceed maximum string length.
    /// @note This creates a new internal buffer; for efficiency in loops, use StringBuilder.
    String& operator+=(const String& other);

    /// @brief Appends a C string to this string.
    /// @param other The C string to append.
    /// @return A reference to this String.
    /// @throws OverflowException If the result would exceed maximum string length.
    /// @note This creates a new internal buffer; for efficiency in loops, use StringBuilder.
    String& operator+=(const char* other);

    /// @brief Appends a character to this string.
    /// @param c The character to append.
    /// @return A reference to this String.
    /// @note This creates a new internal buffer; for efficiency in loops, use StringBuilder.
    String& operator+=(Char c);

    // Static members

    /// @brief Represents the empty string. This field is read-only.
    static const String Empty;

    /// @brief Indicates whether the specified string is null or empty.
    /// @param s The string to test.
    /// @return True if the string is empty; otherwise, false.
    static Boolean IsNullOrEmpty(const String& s);

    /// @brief Indicates whether the specified string is null, empty, or consists only of whitespace characters.
    /// @param s The string to test.
    /// @return True if the string is empty or whitespace only; otherwise, false.
    static Boolean IsNullOrWhiteSpace(const String& s);

    /// @brief Concatenates two strings.
    /// @param s1 The first string.
    /// @param s2 The second string.
    /// @return A new String that is the concatenation of s1 and s2.
    static String Concat(const String& s1, const String& s2);

    /// @brief Concatenates three strings.
    /// @param s1 The first string.
    /// @param s2 The second string.
    /// @param s3 The third string.
    /// @return A new String that is the concatenation of s1, s2, and s3.
    static String Concat(const String& s1, const String& s2, const String& s3);

    /// @brief Compares two strings and returns an integer indicating their relative position in sort order.
    /// @param s1 The first string to compare.
    /// @param s2 The second string to compare.
    /// @return A negative value if s1 precedes s2, zero if they are equal, or a positive value if s1 follows s2.
    static Int32 Compare(const String& s1, const String& s2);

    /// @brief Compares two strings ignoring case and returns an integer indicating their relative position in sort order.
    /// @param s1 The first string to compare.
    /// @param s2 The second string to compare.
    /// @return A negative value if s1 precedes s2, zero if they are equal, or a positive value if s1 follows s2.
    static Int32 CompareIgnoreCase(const String& s1, const String& s2);

    // Comparison

    /// @brief Compares this instance with another string and returns an integer indicating their relative position.
    /// @param other The String to compare to this instance.
    /// @return A negative value if this precedes other, zero if equal, or a positive value if this follows other.
    Int32 CompareTo(const String& other) const;

    /// @brief Determines whether this string and another string have the same value.
    /// @param other The String to compare to this instance.
    /// @return True if the strings are equal; otherwise, false.
    Boolean Equals(const String& other) const;

    /// @brief Determines whether this string and another string have the same value, ignoring case.
    /// @param other The String to compare to this instance.
    /// @return True if the strings are equal ignoring case; otherwise, false.
    Boolean EqualsIgnoreCase(const String& other) const;

    // Hashing (simple)

    /// @brief Returns a hash code for this string.
    /// @return A 32-bit signed integer hash code.
    Int32 GetHashCode() const;
};

// Non-member operators

/// @brief Concatenates a C string with a String.
/// @param lhs The C string (left-hand side).
/// @param rhs The String (right-hand side).
/// @return A new String that is the concatenation of lhs and rhs.
String operator+(const char* lhs, const String& rhs);

/******************************************************************************/
/*    System::StringBuilder                                                    */
/******************************************************************************/

/// @brief Represents a mutable string of characters with efficient append operations.
///
/// Use StringBuilder when concatenating many strings in a loop or building
/// strings incrementally. Unlike String (which is immutable), StringBuilder
/// modifies its internal buffer in-place, avoiding allocations.
///
/// The class uses an internal buffer that grows automatically as needed.
/// For best performance, specify an initial capacity if the final size is known.
///
/// All Append and Insert methods return a reference to the StringBuilder,
/// enabling method chaining:
/// @code
/// StringBuilder sb;
/// sb.Append("Hello").Append(' ').Append("World").AppendLine();
/// String result = sb.ToString();
/// @endcode
///
/// @note This class mirrors the .NET System.Text.StringBuilder API.
class StringBuilder
{
private:
    char* _buffer;
    Int32 _length;      // Current string length
    Int32 _capacity;    // Buffer capacity (always > _length for null terminator)

    static const Int32 DEFAULT_CAPACITY;
    static const Int32 GROWTH_FACTOR;

    void EnsureCapacity(Int32 minCapacity);

public:
    // Constructors

    /// @brief Initializes a new instance of the StringBuilder class with default capacity.
    StringBuilder();

    /// @brief Initializes a new instance of the StringBuilder class with the specified capacity.
    /// @param capacity The initial capacity of the internal buffer.
    /// @throws ArgumentOutOfRangeException If capacity is negative.
    explicit StringBuilder(Int32 capacity);

    /// @brief Initializes a new instance of the StringBuilder class with the specified string value.
    /// @param value The initial string value.
    explicit StringBuilder(const String& value);

    /// @brief Initializes a new instance of the StringBuilder class as a copy of another StringBuilder.
    /// @param other The StringBuilder to copy.
    StringBuilder(const StringBuilder& other);

    /// @brief Initializes a new instance of the StringBuilder class by moving from another StringBuilder.
    /// @param other The StringBuilder to move from. Will be left in an empty state.
    StringBuilder(StringBuilder&& other) noexcept;

    /// @brief Destroys the StringBuilder instance and releases its buffer.
    ~StringBuilder();

    // Assignment

    /// @brief Assigns the value of another StringBuilder to this instance.
    /// @param other The StringBuilder to copy from.
    /// @return A reference to this StringBuilder.
    StringBuilder& operator=(const StringBuilder& other);

    /// @brief Moves the value of another StringBuilder to this instance.
    /// @param other The StringBuilder to move from. Will be left in an empty state.
    /// @return A reference to this StringBuilder.
    StringBuilder& operator=(StringBuilder&& other) noexcept;

    // Properties

    /// @brief Gets the length of the current string value.
    /// @return The number of characters in the current string.
    Int32 Length() const;

    /// @brief Gets the current capacity of the internal buffer.
    /// @return The maximum number of characters that can be stored without reallocation.
    Int32 Capacity() const;

    /// @brief Gets the character at the specified index.
    /// @param index The zero-based index of the character.
    /// @return The character at the specified position.
    /// @throws IndexOutOfRangeException If index is less than zero or greater than or equal to Length().
    Char operator[](Int32 index) const;

    /// @brief Sets the character at the specified index.
    /// @param index The zero-based index of the character to set.
    /// @param c The new character value.
    /// @throws IndexOutOfRangeException If index is less than zero or greater than or equal to Length().
    void SetCharAt(Int32 index, Char c);

    // Append methods (return *this for chaining)

    /// @brief Appends a String to this instance.
    /// @param value The String to append.
    /// @return A reference to this StringBuilder for method chaining.
    StringBuilder& Append(const String& value);

    /// @brief Appends a C string to this instance.
    /// @param value The C string to append.
    /// @return A reference to this StringBuilder for method chaining.
    StringBuilder& Append(const char* value);

    /// @brief Appends a Char to this instance.
    /// @param value The Char to append.
    /// @return A reference to this StringBuilder for method chaining.
    StringBuilder& Append(Char value);

    /// @brief Appends a char to this instance.
    /// @param value The char to append.
    /// @return A reference to this StringBuilder for method chaining.
    StringBuilder& Append(char value);

    /// @brief Appends the string representation of an Int32 to this instance.
    /// @param value The Int32 value to append.
    /// @return A reference to this StringBuilder for method chaining.
    StringBuilder& Append(Int32 value);

    /// @brief Appends the string representation of a Boolean to this instance.
    /// @param value The Boolean value to append ("True" or "False").
    /// @return A reference to this StringBuilder for method chaining.
    StringBuilder& Append(Boolean value);

    /// @brief Appends a newline character to this instance.
    /// @return A reference to this StringBuilder for method chaining.
    StringBuilder& AppendLine();

    /// @brief Appends a String followed by a newline character to this instance.
    /// @param value The String to append before the newline.
    /// @return A reference to this StringBuilder for method chaining.
    StringBuilder& AppendLine(const String& value);

    /// @brief Appends a C string followed by a newline character to this instance.
    /// @param value The C string to append before the newline.
    /// @return A reference to this StringBuilder for method chaining.
    StringBuilder& AppendLine(const char* value);

    // Insert (at position)

    /// @brief Inserts a String at the specified position.
    /// @param index The zero-based position at which to insert.
    /// @param value The String to insert.
    /// @return A reference to this StringBuilder for method chaining.
    /// @throws ArgumentOutOfRangeException If index is less than zero or greater than Length().
    StringBuilder& Insert(Int32 index, const String& value);

    /// @brief Inserts a C string at the specified position.
    /// @param index The zero-based position at which to insert.
    /// @param value The C string to insert.
    /// @return A reference to this StringBuilder for method chaining.
    /// @throws ArgumentOutOfRangeException If index is less than zero or greater than Length().
    StringBuilder& Insert(Int32 index, const char* value);

    /// @brief Inserts a Char at the specified position.
    /// @param index The zero-based position at which to insert.
    /// @param value The Char to insert.
    /// @return A reference to this StringBuilder for method chaining.
    /// @throws ArgumentOutOfRangeException If index is less than zero or greater than Length().
    StringBuilder& Insert(Int32 index, Char value);

    // Remove

    /// @brief Removes a range of characters from this instance.
    /// @param startIndex The zero-based position where removal begins.
    /// @param length The number of characters to remove.
    /// @return A reference to this StringBuilder for method chaining.
    /// @throws ArgumentOutOfRangeException If startIndex or length is negative, or startIndex + length exceeds Length().
    StringBuilder& Remove(Int32 startIndex, Int32 length);

    // Clear

    /// @brief Removes all characters from this instance.
    /// @return A reference to this StringBuilder for method chaining.
    StringBuilder& Clear();

    // Reserve capacity

    /// @brief Ensures that the capacity of this instance is at least the specified value.
    /// @param capacity The minimum capacity to ensure.
    void Reserve(Int32 capacity);

    // Convert to String

    /// @brief Converts the value of this instance to a String.
    /// @return A String whose value is the same as this instance.
    String ToString() const;
};

} // namespace System

#endif // SYSTEM_STRING_HPP
