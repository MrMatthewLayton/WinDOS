/// @file Types.hpp
/// @brief Provides primitive type wrappers for the System namespace.
///
/// This file contains .NET-style wrapper classes for primitive types including
/// Boolean, Char, integer types (Int8-Int64, UInt8-UInt64), and floating-point
/// types (Float32, Float64). These wrappers provide bounds checking, parsing,
/// and string conversion capabilities.

#ifndef SYSTEM_TYPES_HPP
#define SYSTEM_TYPES_HPP

#include "Exception.hpp"
#include <limits>

namespace System
{

// Forward declaration for ToString methods
class String;

// ============================================================================
// Boolean
// ============================================================================

/// @brief Represents a Boolean (true/false) value.
///
/// This class wraps the primitive bool type and provides additional functionality
/// including string parsing and conversion. It is implicitly convertible to and
/// from the primitive bool type.
class Boolean
{
    bool _value;

public:
    /// @brief The Boolean value representing true.
    static const Boolean True;

    /// @brief The Boolean value representing false.
    static const Boolean False;

    /// @brief Initializes a new instance of the Boolean class to false.
    Boolean() : _value(false)
    {
    }

    /// @brief Initializes a new instance of the Boolean class with the specified value.
    /// @param v The boolean value to wrap.
    Boolean(bool v) : _value(v)
    {
    }

    /// @brief Converts this Boolean to a primitive bool.
    /// @return The underlying bool value.
    operator bool() const
    {
        return _value;
    }

    /// @brief Returns the logical negation of this Boolean.
    /// @return A Boolean with the opposite value.
    Boolean operator!() const
    {
        return !_value;
    }

    /// @brief Performs a logical AND operation with another Boolean.
    /// @param other The Boolean to AND with this instance.
    /// @return The result of the logical AND operation.
    Boolean operator&&(const Boolean& other) const
    {
        return _value && other._value;
    }

    /// @brief Performs a logical OR operation with another Boolean.
    /// @param other The Boolean to OR with this instance.
    /// @return The result of the logical OR operation.
    Boolean operator||(const Boolean& other) const
    {
        return _value || other._value;
    }

    /// @brief Performs a logical AND operation with a primitive bool.
    /// @param other The bool value to AND with this instance.
    /// @return The result of the logical AND operation.
    Boolean operator&&(bool other) const
    {
        return _value && other;
    }

    /// @brief Performs a logical OR operation with a primitive bool.
    /// @param other The bool value to OR with this instance.
    /// @return The result of the logical OR operation.
    Boolean operator||(bool other) const
    {
        return _value || other;
    }

    /// @brief Determines whether this Boolean equals another Boolean.
    /// @param other The Boolean to compare with.
    /// @return true if the values are equal; otherwise, false.
    bool operator==(const Boolean& other) const
    {
        return _value == other._value;
    }

    /// @brief Determines whether this Boolean differs from another Boolean.
    /// @param other The Boolean to compare with.
    /// @return true if the values differ; otherwise, false.
    bool operator!=(const Boolean& other) const
    {
        return _value != other._value;
    }

    /// @brief Determines whether this Boolean equals a primitive bool.
    /// @param other The bool value to compare with.
    /// @return true if the values are equal; otherwise, false.
    bool operator==(bool other) const
    {
        return _value == other;
    }

    /// @brief Determines whether this Boolean differs from a primitive bool.
    /// @param other The bool value to compare with.
    /// @return true if the values differ; otherwise, false.
    bool operator!=(bool other) const
    {
        return _value != other;
    }

    /// @brief Converts this Boolean to its string representation.
    /// @return "True" if true, "False" if false.
    String ToString() const;

    /// @brief Parses a string to produce a Boolean value.
    /// @param s The string to parse (case-insensitive "true" or "false").
    /// @return The parsed Boolean value.
    /// @throws FormatException if the string cannot be parsed.
    static Boolean Parse(const String& s);

    /// @brief Attempts to parse a string to a Boolean value.
    /// @param s The string to parse.
    /// @param result When this method returns, contains the parsed Boolean if successful.
    /// @return true if parsing succeeded; otherwise, false.
    static bool TryParse(const String& s, Boolean& result);

    /// @brief Returns a hash code for this Boolean.
    /// @return 1 if true, 0 if false.
    int GetHashCode() const
    {
        return _value ? 1 : 0;
    }
};

// ============================================================================
// Char
// ============================================================================

/// @brief Represents a character as a UTF-8 code unit (ASCII compatible).
///
/// This class wraps the primitive char type and provides character classification
/// and conversion methods. It is implicitly convertible to and from primitive char.
class Char
{
    char _value;

public:
    /// @brief Represents the smallest possible value of a Char (null character).
    static constexpr char MinValue = '\0';

    /// @brief Represents the largest possible value of a Char (DEL character, 0x7F).
    static constexpr char MaxValue = '\x7F';

    /// @brief Initializes a new instance of the Char class to the null character.
    Char() : _value('\0')
    {
    }

    /// @brief Initializes a new instance of the Char class with the specified character.
    /// @param v The character value to wrap.
    Char(char v) : _value(v)
    {
    }

    /// @brief Converts this Char to a primitive char.
    /// @return The underlying char value.
    operator char() const
    {
        return _value;
    }

    /// @brief Determines whether this Char equals another Char.
    /// @param other The Char to compare with.
    /// @return true if the characters are equal; otherwise, false.
    bool operator==(const Char& other) const
    {
        return _value == other._value;
    }

    /// @brief Determines whether this Char differs from another Char.
    /// @param other The Char to compare with.
    /// @return true if the characters differ; otherwise, false.
    bool operator!=(const Char& other) const
    {
        return _value != other._value;
    }

    /// @brief Determines whether this Char is less than another Char.
    /// @param other The Char to compare with.
    /// @return true if this character is less than other; otherwise, false.
    bool operator<(const Char& other) const
    {
        return _value < other._value;
    }

    /// @brief Determines whether this Char is greater than another Char.
    /// @param other The Char to compare with.
    /// @return true if this character is greater than other; otherwise, false.
    bool operator>(const Char& other) const
    {
        return _value > other._value;
    }

    /// @brief Determines whether this Char is less than or equal to another Char.
    /// @param other The Char to compare with.
    /// @return true if this character is less than or equal to other; otherwise, false.
    bool operator<=(const Char& other) const
    {
        return _value <= other._value;
    }

    /// @brief Determines whether this Char is greater than or equal to another Char.
    /// @param other The Char to compare with.
    /// @return true if this character is greater than or equal to other; otherwise, false.
    bool operator>=(const Char& other) const
    {
        return _value >= other._value;
    }

    /// @brief Determines whether this Char equals a primitive char.
    /// @param other The char to compare with.
    /// @return true if the characters are equal; otherwise, false.
    bool operator==(char other) const
    {
        return _value == other;
    }

    /// @brief Determines whether this Char differs from a primitive char.
    /// @param other The char to compare with.
    /// @return true if the characters differ; otherwise, false.
    bool operator!=(char other) const
    {
        return _value != other;
    }

    /// @brief Determines whether this Char is less than a primitive char.
    /// @param other The char to compare with.
    /// @return true if this character is less than other; otherwise, false.
    bool operator<(char other) const
    {
        return _value < other;
    }

    /// @brief Determines whether this Char is greater than a primitive char.
    /// @param other The char to compare with.
    /// @return true if this character is greater than other; otherwise, false.
    bool operator>(char other) const
    {
        return _value > other;
    }

    /// @brief Determines whether this Char is less than or equal to a primitive char.
    /// @param other The char to compare with.
    /// @return true if this character is less than or equal to other; otherwise, false.
    bool operator<=(char other) const
    {
        return _value <= other;
    }

    /// @brief Determines whether this Char is greater than or equal to a primitive char.
    /// @param other The char to compare with.
    /// @return true if this character is greater than or equal to other; otherwise, false.
    bool operator>=(char other) const
    {
        return _value >= other;
    }

    /// @brief Indicates whether the specified character is a decimal digit (0-9).
    /// @param c The character to evaluate.
    /// @return true if c is a digit; otherwise, false.
    static bool IsDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    /// @brief Indicates whether the specified character is a letter (A-Z, a-z).
    /// @param c The character to evaluate.
    /// @return true if c is a letter; otherwise, false.
    static bool IsLetter(char c)
    {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }

    /// @brief Indicates whether the specified character is a letter or digit.
    /// @param c The character to evaluate.
    /// @return true if c is a letter or digit; otherwise, false.
    static bool IsLetterOrDigit(char c)
    {
        return IsLetter(c) || IsDigit(c);
    }

    /// @brief Indicates whether the specified character is white space.
    /// @param c The character to evaluate.
    /// @return true if c is a space, tab, newline, or carriage return; otherwise, false.
    static bool IsWhiteSpace(char c)
    {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    /// @brief Indicates whether the specified character is an uppercase letter.
    /// @param c The character to evaluate.
    /// @return true if c is an uppercase letter (A-Z); otherwise, false.
    static bool IsUpper(char c)
    {
        return c >= 'A' && c <= 'Z';
    }

    /// @brief Indicates whether the specified character is a lowercase letter.
    /// @param c The character to evaluate.
    /// @return true if c is a lowercase letter (a-z); otherwise, false.
    static bool IsLower(char c)
    {
        return c >= 'a' && c <= 'z';
    }

    /// @brief Converts the specified character to uppercase.
    /// @param c The character to convert.
    /// @return The uppercase equivalent of c, or c if it is not a lowercase letter.
    static char ToUpper(char c)
    {
        return IsLower(c) ? (c - 32) : c;
    }

    /// @brief Converts the specified character to lowercase.
    /// @param c The character to convert.
    /// @return The lowercase equivalent of c, or c if it is not an uppercase letter.
    static char ToLower(char c)
    {
        return IsUpper(c) ? (c + 32) : c;
    }

    /// @brief Converts this Char to its string representation.
    /// @return A string containing this single character.
    String ToString() const;

    /// @brief Returns a hash code for this Char.
    /// @return The numeric value of the character.
    int GetHashCode() const
    {
        return static_cast<int>(_value);
    }
};

// ============================================================================
// Integer Types - Macro for common implementation
// ============================================================================

/// @brief Macro that defines an integer wrapper type.
///
/// This macro generates a complete integer wrapper class with:
/// - Constructors (default and from primitive)
/// - Implicit conversion to underlying type
/// - All arithmetic operators (+, -, *, /, %)
/// - Unary operators (+, -, ++, --)
/// - Compound assignment operators (+=, -=, *=, /=, %=)
/// - Bitwise operators (&, |, ^, ~, <<, >>)
/// - Bitwise assignment operators (&=, |=, ^=, <<=, >>=)
/// - Comparison operators (==, !=, <, >, <=, >=)
/// - Overloads for primitive types to avoid ambiguity
/// - ToString, Parse, TryParse methods
/// - GetHashCode method
///
/// @param ClassName The name of the wrapper class to generate.
/// @param UnderlyingType The primitive type to wrap.
/// @param MinVal The minimum value for this type.
/// @param MaxVal The maximum value for this type.
#define DEFINE_INTEGER_TYPE(ClassName, UnderlyingType, MinVal, MaxVal) \
/** @brief Represents a signed/unsigned integer value with bounds checking and string conversion. */ \
/** */ \
/** This class wraps the primitive type and provides additional functionality */ \
/** including division-by-zero protection, parsing, and string conversion. */ \
/** It is implicitly convertible to and from the underlying primitive type. */ \
class ClassName \
{ \
    UnderlyingType _value; \
\
public: \
    /** @brief Represents the smallest possible value of this type. */ \
    static constexpr UnderlyingType MinValue = MinVal; \
    /** @brief Represents the largest possible value of this type. */ \
    static constexpr UnderlyingType MaxValue = MaxVal; \
    \
    /** @brief Initializes a new instance to zero. */ \
    ClassName() : _value(0) \
    { \
    } \
    \
    /** @brief Initializes a new instance with the specified value. */ \
    /** @param v The value to wrap. */ \
    ClassName(UnderlyingType v) : _value(v) \
    { \
    } \
    \
    /** @brief Converts this instance to its underlying primitive type. */ \
    /** @return The underlying value. */ \
    operator UnderlyingType() const \
    { \
        return _value; \
    } \
    \
    /** @brief Adds two values. */ \
    /** @param other The value to add. */ \
    /** @return The sum of the two values. */ \
    ClassName operator+(const ClassName& other) const \
    { \
        return _value + other._value; \
    } \
    \
    /** @brief Subtracts one value from another. */ \
    /** @param other The value to subtract. */ \
    /** @return The difference of the two values. */ \
    ClassName operator-(const ClassName& other) const \
    { \
        return _value - other._value; \
    } \
    \
    /** @brief Multiplies two values. */ \
    /** @param other The value to multiply by. */ \
    /** @return The product of the two values. */ \
    ClassName operator*(const ClassName& other) const \
    { \
        return _value * other._value; \
    } \
    \
    /** @brief Divides one value by another. */ \
    /** @param other The divisor. */ \
    /** @return The quotient of the division. */ \
    /** @throws InvalidOperationException if other is zero. */ \
    ClassName operator/(const ClassName& other) const \
    { \
        if (other._value == 0) \
        { \
            throw InvalidOperationException("Division by zero."); \
        } \
        return _value / other._value; \
    } \
    \
    /** @brief Computes the remainder of division. */ \
    /** @param other The divisor. */ \
    /** @return The remainder after dividing this value by other. */ \
    /** @throws InvalidOperationException if other is zero. */ \
    ClassName operator%(const ClassName& other) const \
    { \
        if (other._value == 0) \
        { \
            throw InvalidOperationException("Division by zero."); \
        } \
        return _value % other._value; \
    } \
    \
    /** @brief Returns the negation of this value. */ \
    /** @return The negated value. */ \
    ClassName operator-() const \
    { \
        return -_value; \
    } \
    \
    /** @brief Returns this value (unary plus). */ \
    /** @return This value unchanged. */ \
    ClassName operator+() const \
    { \
        return +_value; \
    } \
    \
    /** @brief Pre-increments this value. */ \
    /** @return A reference to this instance after incrementing. */ \
    ClassName& operator++() \
    { \
        ++_value; \
        return *this; \
    } \
    \
    /** @brief Post-increments this value. */ \
    /** @return The value before incrementing. */ \
    ClassName operator++(int) \
    { \
        ClassName tmp(*this); \
        ++_value; \
        return tmp; \
    } \
    \
    /** @brief Pre-decrements this value. */ \
    /** @return A reference to this instance after decrementing. */ \
    ClassName& operator--() \
    { \
        --_value; \
        return *this; \
    } \
    \
    /** @brief Post-decrements this value. */ \
    /** @return The value before decrementing. */ \
    ClassName operator--(int) \
    { \
        ClassName tmp(*this); \
        --_value; \
        return tmp; \
    } \
    \
    /** @brief Adds another value to this instance. */ \
    /** @param other The value to add. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator+=(const ClassName& other) \
    { \
        _value += other._value; \
        return *this; \
    } \
    \
    /** @brief Subtracts another value from this instance. */ \
    /** @param other The value to subtract. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator-=(const ClassName& other) \
    { \
        _value -= other._value; \
        return *this; \
    } \
    \
    /** @brief Multiplies this instance by another value. */ \
    /** @param other The value to multiply by. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator*=(const ClassName& other) \
    { \
        _value *= other._value; \
        return *this; \
    } \
    \
    /** @brief Divides this instance by another value. */ \
    /** @param other The divisor. */ \
    /** @return A reference to this instance. */ \
    /** @throws InvalidOperationException if other is zero. */ \
    ClassName& operator/=(const ClassName& other) \
    { \
        if (other._value == 0) \
        { \
            throw InvalidOperationException("Division by zero."); \
        } \
        _value /= other._value; \
        return *this; \
    } \
    \
    /** @brief Computes the remainder and assigns to this instance. */ \
    /** @param other The divisor. */ \
    /** @return A reference to this instance. */ \
    /** @throws InvalidOperationException if other is zero. */ \
    ClassName& operator%=(const ClassName& other) \
    { \
        if (other._value == 0) \
        { \
            throw InvalidOperationException("Division by zero."); \
        } \
        _value %= other._value; \
        return *this; \
    } \
    \
    /** @brief Performs a bitwise AND operation. */ \
    /** @param other The value to AND with. */ \
    /** @return The result of the bitwise AND. */ \
    ClassName operator&(const ClassName& other) const \
    { \
        return _value & other._value; \
    } \
    \
    /** @brief Performs a bitwise OR operation. */ \
    /** @param other The value to OR with. */ \
    /** @return The result of the bitwise OR. */ \
    ClassName operator|(const ClassName& other) const \
    { \
        return _value | other._value; \
    } \
    \
    /** @brief Performs a bitwise XOR operation. */ \
    /** @param other The value to XOR with. */ \
    /** @return The result of the bitwise XOR. */ \
    ClassName operator^(const ClassName& other) const \
    { \
        return _value ^ other._value; \
    } \
    \
    /** @brief Performs a bitwise NOT operation. */ \
    /** @return The bitwise complement of this value. */ \
    ClassName operator~() const \
    { \
        return ~_value; \
    } \
    \
    /** @brief Shifts the value left by the specified number of bits. */ \
    /** @param shift The number of bits to shift. */ \
    /** @return The left-shifted value. */ \
    ClassName operator<<(int shift) const \
    { \
        return _value << shift; \
    } \
    \
    /** @brief Shifts the value right by the specified number of bits. */ \
    /** @param shift The number of bits to shift. */ \
    /** @return The right-shifted value. */ \
    ClassName operator>>(int shift) const \
    { \
        return _value >> shift; \
    } \
    \
    /** @brief Performs bitwise AND and assigns the result. */ \
    /** @param other The value to AND with. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator&=(const ClassName& other) \
    { \
        _value &= other._value; \
        return *this; \
    } \
    \
    /** @brief Performs bitwise OR and assigns the result. */ \
    /** @param other The value to OR with. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator|=(const ClassName& other) \
    { \
        _value |= other._value; \
        return *this; \
    } \
    \
    /** @brief Performs bitwise XOR and assigns the result. */ \
    /** @param other The value to XOR with. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator^=(const ClassName& other) \
    { \
        _value ^= other._value; \
        return *this; \
    } \
    \
    /** @brief Shifts left and assigns the result. */ \
    /** @param shift The number of bits to shift. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator<<=(int shift) \
    { \
        _value <<= shift; \
        return *this; \
    } \
    \
    /** @brief Shifts right and assigns the result. */ \
    /** @param shift The number of bits to shift. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator>>=(int shift) \
    { \
        _value >>= shift; \
        return *this; \
    } \
    \
    /** @brief Determines whether this value equals another. */ \
    /** @param other The value to compare with. */ \
    /** @return true if the values are equal; otherwise, false. */ \
    bool operator==(const ClassName& other) const \
    { \
        return _value == other._value; \
    } \
    \
    /** @brief Determines whether this value differs from another. */ \
    /** @param other The value to compare with. */ \
    /** @return true if the values differ; otherwise, false. */ \
    bool operator!=(const ClassName& other) const \
    { \
        return _value != other._value; \
    } \
    \
    /** @brief Determines whether this value is less than another. */ \
    /** @param other The value to compare with. */ \
    /** @return true if this value is less than other; otherwise, false. */ \
    bool operator<(const ClassName& other) const \
    { \
        return _value < other._value; \
    } \
    \
    /** @brief Determines whether this value is greater than another. */ \
    /** @param other The value to compare with. */ \
    /** @return true if this value is greater than other; otherwise, false. */ \
    bool operator>(const ClassName& other) const \
    { \
        return _value > other._value; \
    } \
    \
    /** @brief Determines whether this value is less than or equal to another. */ \
    /** @param other The value to compare with. */ \
    /** @return true if this value is less than or equal to other; otherwise, false. */ \
    bool operator<=(const ClassName& other) const \
    { \
        return _value <= other._value; \
    } \
    \
    /** @brief Determines whether this value is greater than or equal to another. */ \
    /** @param other The value to compare with. */ \
    /** @return true if this value is greater than or equal to other; otherwise, false. */ \
    bool operator>=(const ClassName& other) const \
    { \
        return _value >= other._value; \
    } \
    \
    /** @brief Determines whether this value equals a primitive value. */ \
    /** @param other The primitive value to compare with. */ \
    /** @return true if the values are equal; otherwise, false. */ \
    bool operator==(UnderlyingType other) const \
    { \
        return _value == other; \
    } \
    \
    /** @brief Determines whether this value differs from a primitive value. */ \
    /** @param other The primitive value to compare with. */ \
    /** @return true if the values differ; otherwise, false. */ \
    bool operator!=(UnderlyingType other) const \
    { \
        return _value != other; \
    } \
    \
    /** @brief Determines whether this value is less than a primitive value. */ \
    /** @param other The primitive value to compare with. */ \
    /** @return true if this value is less than other; otherwise, false. */ \
    bool operator<(UnderlyingType other) const \
    { \
        return _value < other; \
    } \
    \
    /** @brief Determines whether this value is greater than a primitive value. */ \
    /** @param other The primitive value to compare with. */ \
    /** @return true if this value is greater than other; otherwise, false. */ \
    bool operator>(UnderlyingType other) const \
    { \
        return _value > other; \
    } \
    \
    /** @brief Determines whether this value is less than or equal to a primitive value. */ \
    /** @param other The primitive value to compare with. */ \
    /** @return true if this value is less than or equal to other; otherwise, false. */ \
    bool operator<=(UnderlyingType other) const \
    { \
        return _value <= other; \
    } \
    \
    /** @brief Determines whether this value is greater than or equal to a primitive value. */ \
    /** @param other The primitive value to compare with. */ \
    /** @return true if this value is greater than or equal to other; otherwise, false. */ \
    bool operator>=(UnderlyingType other) const \
    { \
        return _value >= other; \
    } \
    \
    /** @brief Adds a primitive value to this instance. */ \
    /** @param other The primitive value to add. */ \
    /** @return The sum of the two values. */ \
    ClassName operator+(UnderlyingType other) const \
    { \
        return _value + other; \
    } \
    \
    /** @brief Subtracts a primitive value from this instance. */ \
    /** @param other The primitive value to subtract. */ \
    /** @return The difference of the two values. */ \
    ClassName operator-(UnderlyingType other) const \
    { \
        return _value - other; \
    } \
    \
    /** @brief Multiplies this instance by a primitive value. */ \
    /** @param other The primitive value to multiply by. */ \
    /** @return The product of the two values. */ \
    ClassName operator*(UnderlyingType other) const \
    { \
        return _value * other; \
    } \
    \
    /** @brief Divides this instance by a primitive value. */ \
    /** @param other The primitive divisor. */ \
    /** @return The quotient of the division. */ \
    /** @throws InvalidOperationException if other is zero. */ \
    ClassName operator/(UnderlyingType other) const \
    { \
        if (other == 0) \
        { \
            throw InvalidOperationException("Division by zero."); \
        } \
        return _value / other; \
    } \
    \
    /** @brief Computes the remainder of division by a primitive value. */ \
    /** @param other The primitive divisor. */ \
    /** @return The remainder after division. */ \
    /** @throws InvalidOperationException if other is zero. */ \
    ClassName operator%(UnderlyingType other) const \
    { \
        if (other == 0) \
        { \
            throw InvalidOperationException("Division by zero."); \
        } \
        return _value % other; \
    } \
    \
    /** @brief Adds a primitive value and assigns the result. */ \
    /** @param other The primitive value to add. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator+=(UnderlyingType other) \
    { \
        _value += other; \
        return *this; \
    } \
    \
    /** @brief Subtracts a primitive value and assigns the result. */ \
    /** @param other The primitive value to subtract. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator-=(UnderlyingType other) \
    { \
        _value -= other; \
        return *this; \
    } \
    \
    /** @brief Multiplies by a primitive value and assigns the result. */ \
    /** @param other The primitive value to multiply by. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator*=(UnderlyingType other) \
    { \
        _value *= other; \
        return *this; \
    } \
    \
    /** @brief Divides by a primitive value and assigns the result. */ \
    /** @param other The primitive divisor. */ \
    /** @return A reference to this instance. */ \
    /** @throws InvalidOperationException if other is zero. */ \
    ClassName& operator/=(UnderlyingType other) \
    { \
        if (other == 0) \
        { \
            throw InvalidOperationException("Division by zero."); \
        } \
        _value /= other; \
        return *this; \
    } \
    \
    /** @brief Computes remainder by a primitive value and assigns the result. */ \
    /** @param other The primitive divisor. */ \
    /** @return A reference to this instance. */ \
    /** @throws InvalidOperationException if other is zero. */ \
    ClassName& operator%=(UnderlyingType other) \
    { \
        if (other == 0) \
        { \
            throw InvalidOperationException("Division by zero."); \
        } \
        _value %= other; \
        return *this; \
    } \
    \
    /** @brief Converts this value to its string representation. */ \
    /** @return A string containing the decimal representation of this value. */ \
    String ToString() const; \
    /** @brief Parses a string to produce a value of this type. */ \
    /** @param s The string to parse. */ \
    /** @return The parsed value. */ \
    /** @throws FormatException if the string cannot be parsed. */ \
    /** @throws OverflowException if the parsed value exceeds the type's range. */ \
    static ClassName Parse(const String& s); \
    /** @brief Attempts to parse a string to a value of this type. */ \
    /** @param s The string to parse. */ \
    /** @param result When this method returns, contains the parsed value if successful. */ \
    /** @return true if parsing succeeded; otherwise, false. */ \
    static bool TryParse(const String& s, ClassName& result); \
    \
    /** @brief Returns a hash code for this value. */ \
    /** @return A hash code based on the underlying value. */ \
    int GetHashCode() const \
    { \
        return static_cast<int>(_value); \
    } \
};

/// @brief Represents an 8-bit signed integer (-128 to 127).
DEFINE_INTEGER_TYPE(Int8, signed char, -128, 127)

/// @brief Represents an 8-bit unsigned integer (0 to 255).
DEFINE_INTEGER_TYPE(UInt8, unsigned char, 0, 255)

/// @brief Represents a 16-bit signed integer (-32,768 to 32,767).
DEFINE_INTEGER_TYPE(Int16, short, -32768, 32767)

/// @brief Represents a 16-bit unsigned integer (0 to 65,535).
DEFINE_INTEGER_TYPE(UInt16, unsigned short, 0, 65535)

/// @brief Represents a 32-bit signed integer (-2,147,483,648 to 2,147,483,647).
DEFINE_INTEGER_TYPE(Int32, int, (-2147483647 - 1), 2147483647)

/// @brief Represents a 32-bit unsigned integer (0 to 4,294,967,295).
DEFINE_INTEGER_TYPE(UInt32, unsigned int, 0, 4294967295U)

/// @brief Represents a 64-bit signed integer.
DEFINE_INTEGER_TYPE(Int64, long long, (-9223372036854775807LL - 1), 9223372036854775807LL)

/// @brief Represents a 64-bit unsigned integer.
DEFINE_INTEGER_TYPE(UInt64, unsigned long long, 0, 18446744073709551615ULL)

#undef DEFINE_INTEGER_TYPE

// ============================================================================
// Floating Point Types
// ============================================================================

/// @brief Macro that defines a floating-point wrapper type.
///
/// This macro generates a complete floating-point wrapper class with:
/// - Constructors (default and from primitive)
/// - Implicit conversion to underlying type
/// - Arithmetic operators (+, -, *, /)
/// - Unary operators (+, -)
/// - Compound assignment operators (+=, -=, *=, /=)
/// - Comparison operators (==, !=, <, >, <=, >=)
/// - Overloads for primitive types to avoid ambiguity
/// - Special value checks (IsNaN, IsInfinity, etc.)
/// - ToString, Parse, TryParse methods
/// - GetHashCode method
///
/// @param ClassName The name of the wrapper class to generate.
/// @param UnderlyingType The primitive floating-point type to wrap.
#define DEFINE_FLOAT_TYPE(ClassName, UnderlyingType) \
/** @brief Represents a floating-point value with special value detection and string conversion. */ \
/** */ \
/** This class wraps the primitive floating-point type and provides additional */ \
/** functionality including NaN/Infinity detection, parsing, and string conversion. */ \
/** It is implicitly convertible to and from the underlying primitive type. */ \
class ClassName \
{ \
    UnderlyingType _value; \
\
public: \
    /** @brief Represents the smallest possible value of this type (most negative). */ \
    static constexpr UnderlyingType MinValue = std::numeric_limits<UnderlyingType>::lowest(); \
    /** @brief Represents the largest possible value of this type. */ \
    static constexpr UnderlyingType MaxValue = std::numeric_limits<UnderlyingType>::max(); \
    /** @brief Represents the smallest positive value greater than zero. */ \
    static constexpr UnderlyingType Epsilon = std::numeric_limits<UnderlyingType>::epsilon(); \
    \
    /** @brief Initializes a new instance to zero. */ \
    ClassName() : _value(0) \
    { \
    } \
    \
    /** @brief Initializes a new instance with the specified value. */ \
    /** @param v The value to wrap. */ \
    ClassName(UnderlyingType v) : _value(v) \
    { \
    } \
    \
    /** @brief Converts this instance to its underlying primitive type. */ \
    /** @return The underlying value. */ \
    operator UnderlyingType() const \
    { \
        return _value; \
    } \
    \
    /** @brief Adds two values. */ \
    /** @param other The value to add. */ \
    /** @return The sum of the two values. */ \
    ClassName operator+(const ClassName& other) const \
    { \
        return _value + other._value; \
    } \
    \
    /** @brief Subtracts one value from another. */ \
    /** @param other The value to subtract. */ \
    /** @return The difference of the two values. */ \
    ClassName operator-(const ClassName& other) const \
    { \
        return _value - other._value; \
    } \
    \
    /** @brief Multiplies two values. */ \
    /** @param other The value to multiply by. */ \
    /** @return The product of the two values. */ \
    ClassName operator*(const ClassName& other) const \
    { \
        return _value * other._value; \
    } \
    \
    /** @brief Divides one value by another. */ \
    /** @param other The divisor. */ \
    /** @return The quotient of the division (may be Infinity if other is zero). */ \
    ClassName operator/(const ClassName& other) const \
    { \
        return _value / other._value; \
    } \
    \
    /** @brief Returns the negation of this value. */ \
    /** @return The negated value. */ \
    ClassName operator-() const \
    { \
        return -_value; \
    } \
    \
    /** @brief Returns this value (unary plus). */ \
    /** @return This value unchanged. */ \
    ClassName operator+() const \
    { \
        return +_value; \
    } \
    \
    /** @brief Adds another value to this instance. */ \
    /** @param other The value to add. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator+=(const ClassName& other) \
    { \
        _value += other._value; \
        return *this; \
    } \
    \
    /** @brief Subtracts another value from this instance. */ \
    /** @param other The value to subtract. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator-=(const ClassName& other) \
    { \
        _value -= other._value; \
        return *this; \
    } \
    \
    /** @brief Multiplies this instance by another value. */ \
    /** @param other The value to multiply by. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator*=(const ClassName& other) \
    { \
        _value *= other._value; \
        return *this; \
    } \
    \
    /** @brief Divides this instance by another value. */ \
    /** @param other The divisor. */ \
    /** @return A reference to this instance. */ \
    ClassName& operator/=(const ClassName& other) \
    { \
        _value /= other._value; \
        return *this; \
    } \
    \
    /** @brief Determines whether this value equals another. */ \
    /** @param other The value to compare with. */ \
    /** @return true if the values are equal; otherwise, false. */ \
    bool operator==(const ClassName& other) const \
    { \
        return _value == other._value; \
    } \
    \
    /** @brief Determines whether this value differs from another. */ \
    /** @param other The value to compare with. */ \
    /** @return true if the values differ; otherwise, false. */ \
    bool operator!=(const ClassName& other) const \
    { \
        return _value != other._value; \
    } \
    \
    /** @brief Determines whether this value is less than another. */ \
    /** @param other The value to compare with. */ \
    /** @return true if this value is less than other; otherwise, false. */ \
    bool operator<(const ClassName& other) const \
    { \
        return _value < other._value; \
    } \
    \
    /** @brief Determines whether this value is greater than another. */ \
    /** @param other The value to compare with. */ \
    /** @return true if this value is greater than other; otherwise, false. */ \
    bool operator>(const ClassName& other) const \
    { \
        return _value > other._value; \
    } \
    \
    /** @brief Determines whether this value is less than or equal to another. */ \
    /** @param other The value to compare with. */ \
    /** @return true if this value is less than or equal to other; otherwise, false. */ \
    bool operator<=(const ClassName& other) const \
    { \
        return _value <= other._value; \
    } \
    \
    /** @brief Determines whether this value is greater than or equal to another. */ \
    /** @param other The value to compare with. */ \
    /** @return true if this value is greater than or equal to other; otherwise, false. */ \
    bool operator>=(const ClassName& other) const \
    { \
        return _value >= other._value; \
    } \
    \
    /** @brief Determines whether this value equals a primitive value. */ \
    /** @param other The primitive value to compare with. */ \
    /** @return true if the values are equal; otherwise, false. */ \
    bool operator==(UnderlyingType other) const \
    { \
        return _value == other; \
    } \
    \
    /** @brief Determines whether this value differs from a primitive value. */ \
    /** @param other The primitive value to compare with. */ \
    /** @return true if the values differ; otherwise, false. */ \
    bool operator!=(UnderlyingType other) const \
    { \
        return _value != other; \
    } \
    \
    /** @brief Determines whether this value is less than a primitive value. */ \
    /** @param other The primitive value to compare with. */ \
    /** @return true if this value is less than other; otherwise, false. */ \
    bool operator<(UnderlyingType other) const \
    { \
        return _value < other; \
    } \
    \
    /** @brief Determines whether this value is greater than a primitive value. */ \
    /** @param other The primitive value to compare with. */ \
    /** @return true if this value is greater than other; otherwise, false. */ \
    bool operator>(UnderlyingType other) const \
    { \
        return _value > other; \
    } \
    \
    /** @brief Determines whether this value is less than or equal to a primitive value. */ \
    /** @param other The primitive value to compare with. */ \
    /** @return true if this value is less than or equal to other; otherwise, false. */ \
    bool operator<=(UnderlyingType other) const \
    { \
        return _value <= other; \
    } \
    \
    /** @brief Determines whether this value is greater than or equal to a primitive value. */ \
    /** @param other The primitive value to compare with. */ \
    /** @return true if this value is greater than or equal to other; otherwise, false. */ \
    bool operator>=(UnderlyingType other) const \
    { \
        return _value >= other; \
    } \
    \
    /** @brief Indicates whether the specified value is not a number (NaN). */ \
    /** @param value The value to test. */ \
    /** @return true if the value is NaN; otherwise, false. */ \
    static bool IsNaN(ClassName value); \
    /** @brief Indicates whether the specified value is positive or negative infinity. */ \
    /** @param value The value to test. */ \
    /** @return true if the value is infinite; otherwise, false. */ \
    static bool IsInfinity(ClassName value); \
    /** @brief Indicates whether the specified value is positive infinity. */ \
    /** @param value The value to test. */ \
    /** @return true if the value is positive infinity; otherwise, false. */ \
    static bool IsPositiveInfinity(ClassName value); \
    /** @brief Indicates whether the specified value is negative infinity. */ \
    /** @param value The value to test. */ \
    /** @return true if the value is negative infinity; otherwise, false. */ \
    static bool IsNegativeInfinity(ClassName value); \
    \
    /** @brief Converts this value to its string representation. */ \
    /** @return A string containing the decimal representation of this value. */ \
    String ToString() const; \
    /** @brief Parses a string to produce a value of this type. */ \
    /** @param s The string to parse. */ \
    /** @return The parsed value. */ \
    /** @throws FormatException if the string cannot be parsed. */ \
    static ClassName Parse(const String& s); \
    /** @brief Attempts to parse a string to a value of this type. */ \
    /** @param s The string to parse. */ \
    /** @param result When this method returns, contains the parsed value if successful. */ \
    /** @return true if parsing succeeded; otherwise, false. */ \
    static bool TryParse(const String& s, ClassName& result); \
    \
    /** @brief Returns a hash code for this value. */ \
    /** @return A hash code based on the underlying value's bit representation. */ \
    int GetHashCode() const; \
};

/// @brief Represents a single-precision (32-bit) floating-point number.
DEFINE_FLOAT_TYPE(Float32, float)

/// @brief Represents a double-precision (64-bit) floating-point number.
DEFINE_FLOAT_TYPE(Float64, double)

#undef DEFINE_FLOAT_TYPE

// ============================================================================
// Type aliases for compatibility
// ============================================================================

/// @brief Alias for UInt8, representing a byte (0 to 255).
using Byte = UInt8;

/// @brief Alias for Int8, representing a signed byte (-128 to 127).
using SByte = Int8;

/// @brief Alias for Int16, representing a short integer.
using Short = Int16;

/// @brief Alias for UInt16, representing an unsigned short integer.
using UShort = UInt16;

/// @brief Alias for Int32, representing a standard integer.
using Int = Int32;

/// @brief Alias for UInt32, representing an unsigned integer.
using UInt = UInt32;

/// @brief Alias for Int64, representing a long integer.
using Long = Int64;

/// @brief Alias for UInt64, representing an unsigned long integer.
using ULong = UInt64;

/// @brief Alias for Float32, representing a single-precision float.
using Single = Float32;

/// @brief Alias for Float64, representing a double-precision float.
using Double = Float64;

} // namespace System

// Now include String.hpp - it can use the types defined above
#include "String.hpp"

#endif // SYSTEM_TYPES_HPP
