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

class Boolean
{
private:
    bool _value;

public:
    static const Boolean True;
    static const Boolean False;

    Boolean() : _value(false)
    {
    }

    Boolean(bool v) : _value(v)
    {
    }

    operator bool() const
    {
        return _value;
    }

    Boolean operator!() const
    {
        return !_value;
    }

    Boolean operator&&(const Boolean& other) const
    {
        return _value && other._value;
    }

    Boolean operator||(const Boolean& other) const
    {
        return _value || other._value;
    }

    // Overloads with primitive bool to avoid ambiguity
    Boolean operator&&(bool other) const
    {
        return _value && other;
    }

    Boolean operator||(bool other) const
    {
        return _value || other;
    }

    bool operator==(const Boolean& other) const
    {
        return _value == other._value;
    }

    bool operator!=(const Boolean& other) const
    {
        return _value != other._value;
    }

    // Overloads with primitive bool to avoid ambiguity
    bool operator==(bool other) const
    {
        return _value == other;
    }

    bool operator!=(bool other) const
    {
        return _value != other;
    }

    String ToString() const;
    static Boolean Parse(const String& s);
    static bool TryParse(const String& s, Boolean& result);

    int GetHashCode() const
    {
        return _value ? 1 : 0;
    }
};

// ============================================================================
// Char
// ============================================================================

class Char
{
private:
    char _value;

public:
    static constexpr char MinValue = '\0';
    static constexpr char MaxValue = '\x7F';

    Char() : _value('\0')
    {
    }

    Char(char v) : _value(v)
    {
    }

    operator char() const
    {
        return _value;
    }

    bool operator==(const Char& other) const
    {
        return _value == other._value;
    }

    bool operator!=(const Char& other) const
    {
        return _value != other._value;
    }

    bool operator<(const Char& other) const
    {
        return _value < other._value;
    }

    bool operator>(const Char& other) const
    {
        return _value > other._value;
    }

    bool operator<=(const Char& other) const
    {
        return _value <= other._value;
    }

    bool operator>=(const Char& other) const
    {
        return _value >= other._value;
    }

    // Overloads with primitive char to avoid ambiguity
    bool operator==(char other) const
    {
        return _value == other;
    }

    bool operator!=(char other) const
    {
        return _value != other;
    }

    bool operator<(char other) const
    {
        return _value < other;
    }

    bool operator>(char other) const
    {
        return _value > other;
    }

    bool operator<=(char other) const
    {
        return _value <= other;
    }

    bool operator>=(char other) const
    {
        return _value >= other;
    }

    static bool IsDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    static bool IsLetter(char c)
    {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }

    static bool IsLetterOrDigit(char c)
    {
        return IsLetter(c) || IsDigit(c);
    }

    static bool IsWhiteSpace(char c)
    {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    static bool IsUpper(char c)
    {
        return c >= 'A' && c <= 'Z';
    }

    static bool IsLower(char c)
    {
        return c >= 'a' && c <= 'z';
    }

    static char ToUpper(char c)
    {
        return IsLower(c) ? (c - 32) : c;
    }

    static char ToLower(char c)
    {
        return IsUpper(c) ? (c + 32) : c;
    }

    String ToString() const;

    int GetHashCode() const
    {
        return static_cast<int>(_value);
    }
};

// ============================================================================
// Integer Types - Macro for common implementation
// ============================================================================

#define DEFINE_INTEGER_TYPE(ClassName, UnderlyingType, MinVal, MaxVal) \
class ClassName \
{ \
private: \
    UnderlyingType _value; \
public: \
    static constexpr UnderlyingType MinValue = MinVal; \
    static constexpr UnderlyingType MaxValue = MaxVal; \
    \
    ClassName() : _value(0) \
    { \
    } \
    \
    ClassName(UnderlyingType v) : _value(v) \
    { \
    } \
    \
    operator UnderlyingType() const \
    { \
        return _value; \
    } \
    \
    ClassName operator+(const ClassName& other) const \
    { \
        return _value + other._value; \
    } \
    \
    ClassName operator-(const ClassName& other) const \
    { \
        return _value - other._value; \
    } \
    \
    ClassName operator*(const ClassName& other) const \
    { \
        return _value * other._value; \
    } \
    \
    ClassName operator/(const ClassName& other) const \
    { \
        if (other._value == 0) \
        { \
            throw InvalidOperationException("Division by zero."); \
        } \
        return _value / other._value; \
    } \
    \
    ClassName operator%(const ClassName& other) const \
    { \
        if (other._value == 0) \
        { \
            throw InvalidOperationException("Division by zero."); \
        } \
        return _value % other._value; \
    } \
    \
    ClassName operator-() const \
    { \
        return -_value; \
    } \
    \
    ClassName operator+() const \
    { \
        return +_value; \
    } \
    \
    ClassName& operator++() \
    { \
        ++_value; \
        return *this; \
    } \
    \
    ClassName operator++(int) \
    { \
        ClassName tmp(*this); \
        ++_value; \
        return tmp; \
    } \
    \
    ClassName& operator--() \
    { \
        --_value; \
        return *this; \
    } \
    \
    ClassName operator--(int) \
    { \
        ClassName tmp(*this); \
        --_value; \
        return tmp; \
    } \
    \
    ClassName& operator+=(const ClassName& other) \
    { \
        _value += other._value; \
        return *this; \
    } \
    \
    ClassName& operator-=(const ClassName& other) \
    { \
        _value -= other._value; \
        return *this; \
    } \
    \
    ClassName& operator*=(const ClassName& other) \
    { \
        _value *= other._value; \
        return *this; \
    } \
    \
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
    ClassName operator&(const ClassName& other) const \
    { \
        return _value & other._value; \
    } \
    \
    ClassName operator|(const ClassName& other) const \
    { \
        return _value | other._value; \
    } \
    \
    ClassName operator^(const ClassName& other) const \
    { \
        return _value ^ other._value; \
    } \
    \
    ClassName operator~() const \
    { \
        return ~_value; \
    } \
    \
    ClassName operator<<(int shift) const \
    { \
        return _value << shift; \
    } \
    \
    ClassName operator>>(int shift) const \
    { \
        return _value >> shift; \
    } \
    \
    ClassName& operator&=(const ClassName& other) \
    { \
        _value &= other._value; \
        return *this; \
    } \
    \
    ClassName& operator|=(const ClassName& other) \
    { \
        _value |= other._value; \
        return *this; \
    } \
    \
    ClassName& operator^=(const ClassName& other) \
    { \
        _value ^= other._value; \
        return *this; \
    } \
    \
    ClassName& operator<<=(int shift) \
    { \
        _value <<= shift; \
        return *this; \
    } \
    \
    ClassName& operator>>=(int shift) \
    { \
        _value >>= shift; \
        return *this; \
    } \
    \
    bool operator==(const ClassName& other) const \
    { \
        return _value == other._value; \
    } \
    \
    bool operator!=(const ClassName& other) const \
    { \
        return _value != other._value; \
    } \
    \
    bool operator<(const ClassName& other) const \
    { \
        return _value < other._value; \
    } \
    \
    bool operator>(const ClassName& other) const \
    { \
        return _value > other._value; \
    } \
    \
    bool operator<=(const ClassName& other) const \
    { \
        return _value <= other._value; \
    } \
    \
    bool operator>=(const ClassName& other) const \
    { \
        return _value >= other._value; \
    } \
    \
    /* Comparison with primitive type to avoid ambiguity */ \
    bool operator==(UnderlyingType other) const \
    { \
        return _value == other; \
    } \
    \
    bool operator!=(UnderlyingType other) const \
    { \
        return _value != other; \
    } \
    \
    bool operator<(UnderlyingType other) const \
    { \
        return _value < other; \
    } \
    \
    bool operator>(UnderlyingType other) const \
    { \
        return _value > other; \
    } \
    \
    bool operator<=(UnderlyingType other) const \
    { \
        return _value <= other; \
    } \
    \
    bool operator>=(UnderlyingType other) const \
    { \
        return _value >= other; \
    } \
    \
    /* Arithmetic with primitive type to avoid ambiguity */ \
    ClassName operator+(UnderlyingType other) const \
    { \
        return _value + other; \
    } \
    \
    ClassName operator-(UnderlyingType other) const \
    { \
        return _value - other; \
    } \
    \
    ClassName operator*(UnderlyingType other) const \
    { \
        return _value * other; \
    } \
    \
    ClassName operator/(UnderlyingType other) const \
    { \
        if (other == 0) \
        { \
            throw InvalidOperationException("Division by zero."); \
        } \
        return _value / other; \
    } \
    \
    ClassName operator%(UnderlyingType other) const \
    { \
        if (other == 0) \
        { \
            throw InvalidOperationException("Division by zero."); \
        } \
        return _value % other; \
    } \
    \
    ClassName& operator+=(UnderlyingType other) \
    { \
        _value += other; \
        return *this; \
    } \
    \
    ClassName& operator-=(UnderlyingType other) \
    { \
        _value -= other; \
        return *this; \
    } \
    \
    ClassName& operator*=(UnderlyingType other) \
    { \
        _value *= other; \
        return *this; \
    } \
    \
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
    String ToString() const; \
    static ClassName Parse(const String& s); \
    static bool TryParse(const String& s, ClassName& result); \
    \
    int GetHashCode() const \
    { \
        return static_cast<int>(_value); \
    } \
};

// 8-bit types
DEFINE_INTEGER_TYPE(Int8, signed char, -128, 127)
DEFINE_INTEGER_TYPE(UInt8, unsigned char, 0, 255)

// 16-bit types
DEFINE_INTEGER_TYPE(Int16, short, -32768, 32767)
DEFINE_INTEGER_TYPE(UInt16, unsigned short, 0, 65535)

// 32-bit types
DEFINE_INTEGER_TYPE(Int32, int, (-2147483647 - 1), 2147483647)
DEFINE_INTEGER_TYPE(UInt32, unsigned int, 0, 4294967295U)

// 64-bit types
DEFINE_INTEGER_TYPE(Int64, long long, (-9223372036854775807LL - 1), 9223372036854775807LL)
DEFINE_INTEGER_TYPE(UInt64, unsigned long long, 0, 18446744073709551615ULL)

#undef DEFINE_INTEGER_TYPE

// ============================================================================
// Floating Point Types
// ============================================================================

#define DEFINE_FLOAT_TYPE(ClassName, UnderlyingType) \
class ClassName \
{ \
private: \
    UnderlyingType _value; \
public: \
    static constexpr UnderlyingType MinValue = std::numeric_limits<UnderlyingType>::lowest(); \
    static constexpr UnderlyingType MaxValue = std::numeric_limits<UnderlyingType>::max(); \
    static constexpr UnderlyingType Epsilon = std::numeric_limits<UnderlyingType>::epsilon(); \
    \
    ClassName() : _value(0) \
    { \
    } \
    \
    ClassName(UnderlyingType v) : _value(v) \
    { \
    } \
    \
    operator UnderlyingType() const \
    { \
        return _value; \
    } \
    \
    ClassName operator+(const ClassName& other) const \
    { \
        return _value + other._value; \
    } \
    \
    ClassName operator-(const ClassName& other) const \
    { \
        return _value - other._value; \
    } \
    \
    ClassName operator*(const ClassName& other) const \
    { \
        return _value * other._value; \
    } \
    \
    ClassName operator/(const ClassName& other) const \
    { \
        return _value / other._value; \
    } \
    \
    ClassName operator-() const \
    { \
        return -_value; \
    } \
    \
    ClassName operator+() const \
    { \
        return +_value; \
    } \
    \
    ClassName& operator+=(const ClassName& other) \
    { \
        _value += other._value; \
        return *this; \
    } \
    \
    ClassName& operator-=(const ClassName& other) \
    { \
        _value -= other._value; \
        return *this; \
    } \
    \
    ClassName& operator*=(const ClassName& other) \
    { \
        _value *= other._value; \
        return *this; \
    } \
    \
    ClassName& operator/=(const ClassName& other) \
    { \
        _value /= other._value; \
        return *this; \
    } \
    \
    bool operator==(const ClassName& other) const \
    { \
        return _value == other._value; \
    } \
    \
    bool operator!=(const ClassName& other) const \
    { \
        return _value != other._value; \
    } \
    \
    bool operator<(const ClassName& other) const \
    { \
        return _value < other._value; \
    } \
    \
    bool operator>(const ClassName& other) const \
    { \
        return _value > other._value; \
    } \
    \
    bool operator<=(const ClassName& other) const \
    { \
        return _value <= other._value; \
    } \
    \
    bool operator>=(const ClassName& other) const \
    { \
        return _value >= other._value; \
    } \
    \
    /* Comparison with primitive type to avoid ambiguity */ \
    bool operator==(UnderlyingType other) const \
    { \
        return _value == other; \
    } \
    \
    bool operator!=(UnderlyingType other) const \
    { \
        return _value != other; \
    } \
    \
    bool operator<(UnderlyingType other) const \
    { \
        return _value < other; \
    } \
    \
    bool operator>(UnderlyingType other) const \
    { \
        return _value > other; \
    } \
    \
    bool operator<=(UnderlyingType other) const \
    { \
        return _value <= other; \
    } \
    \
    bool operator>=(UnderlyingType other) const \
    { \
        return _value >= other; \
    } \
    \
    static bool IsNaN(ClassName value); \
    static bool IsInfinity(ClassName value); \
    static bool IsPositiveInfinity(ClassName value); \
    static bool IsNegativeInfinity(ClassName value); \
    \
    String ToString() const; \
    static ClassName Parse(const String& s); \
    static bool TryParse(const String& s, ClassName& result); \
    \
    int GetHashCode() const; \
};

DEFINE_FLOAT_TYPE(Float32, float)
DEFINE_FLOAT_TYPE(Float64, double)

#undef DEFINE_FLOAT_TYPE

// ============================================================================
// Type aliases for compatibility
// ============================================================================

using Byte = UInt8;
using SByte = Int8;
using Short = Int16;
using UShort = UInt16;
using Int = Int32;
using UInt = UInt32;
using Long = Int64;
using ULong = UInt64;
using Single = Float32;
using Double = Float64;

} // namespace System

// Now include String.hpp - it can use the types defined above
#include "String.hpp"

#endif // SYSTEM_TYPES_HPP
