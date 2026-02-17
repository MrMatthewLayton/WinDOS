#include "Types.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

namespace System {

// ============================================================================
// Boolean implementation
// ============================================================================

const Boolean Boolean::True(true);
const Boolean Boolean::False(false);

String Boolean::ToString() const {
    return _value ? String("True") : String("False");
}

Boolean Boolean::Parse(const String& s) {
    Boolean result;
    if (!TryParse(s, result)) {
        throw FormatException("String was not recognized as a valid Boolean.");
    }
    return result;
}

bool Boolean::TryParse(const String& s, Boolean& result) {
    String trimmed = s.Trim();

    if (trimmed.EqualsIgnoreCase("true") || trimmed == "1") {
        result = true;
        return true;
    }
    if (trimmed.EqualsIgnoreCase("false") || trimmed == "0") {
        result = false;
        return true;
    }
    return false;
}

// ============================================================================
// Char implementation
// ============================================================================

String Char::ToString() const {
    return String(_value, 1);
}

// ============================================================================
// Helper functions for integer parsing/formatting
// ============================================================================

namespace {

// Convert integer to string (works for both signed and unsigned)
template<typename T>
String IntToString(T value) {
    if (value == 0) {
        return String("0");
    }

    char buffer[24];  // Enough for 64-bit integers
    int pos = 23;
    buffer[pos] = '\0';

    bool negative = false;
    if (value < 0) {
        negative = true;
        // Handle minimum value edge case
        if (value == std::numeric_limits<T>::min()) {
            // Special handling for minimum negative value
            T lastDigit = -(value % 10);
            value = -(value / 10);
            buffer[--pos] = '0' + static_cast<char>(lastDigit);
        } else {
            value = -value;
        }
    }

    while (value > 0) {
        buffer[--pos] = '0' + (value % 10);
        value /= 10;
    }

    if (negative) {
        buffer[--pos] = '-';
    }

    return String(&buffer[pos]);
}

// Convert unsigned integer to string
template<typename T>
String UIntToString(T value) {
    if (value == 0) {
        return String("0");
    }

    char buffer[24];
    int pos = 23;
    buffer[pos] = '\0';

    while (value > 0) {
        buffer[--pos] = '0' + (value % 10);
        value /= 10;
    }

    return String(&buffer[pos]);
}

// Parse signed integer
template<typename T>
bool TryParseInt(const String& s, T& result, T minVal, T maxVal) {
    if (s.Length() == 0) {
        return false;
    }

    const char* str = s.CStr();
    int i = 0;
    bool negative = false;

    // Skip leading whitespace
    while (str[i] && (str[i] == ' ' || str[i] == '\t')) {
        i++;
    }

    // Handle sign
    if (str[i] == '-') {
        negative = true;
        i++;
    } else if (str[i] == '+') {
        i++;
    }

    if (!str[i]) {
        return false;
    }

    // Parse digits
    long long value = 0;
    while (str[i]) {
        if (str[i] >= '0' && str[i] <= '9') {
            value = value * 10 + (str[i] - '0');

            // Check overflow
            if (negative) {
                if (-value < minVal) return false;
            } else {
                if (value > maxVal) return false;
            }
        } else if (str[i] == ' ' || str[i] == '\t') {
            // Skip trailing whitespace
            i++;
            while (str[i] && (str[i] == ' ' || str[i] == '\t')) {
                i++;
            }
            if (str[i]) return false;  // Non-whitespace after number
            break;
        } else {
            return false;
        }
        i++;
    }

    result = static_cast<T>(negative ? -value : value);
    return true;
}

// Parse unsigned integer
template<typename T>
bool TryParseUInt(const String& s, T& result, T maxVal) {
    if (s.Length() == 0) {
        return false;
    }

    const char* str = s.CStr();
    int i = 0;

    // Skip leading whitespace
    while (str[i] && (str[i] == ' ' || str[i] == '\t')) {
        i++;
    }

    // Handle optional plus sign
    if (str[i] == '+') {
        i++;
    }

    // Negative not allowed for unsigned
    if (str[i] == '-') {
        return false;
    }

    if (!str[i]) {
        return false;
    }

    // Parse digits
    unsigned long long value = 0;
    while (str[i]) {
        if (str[i] >= '0' && str[i] <= '9') {
            unsigned long long newValue = value * 10 + (str[i] - '0');
            if (newValue < value || newValue > maxVal) {
                return false;  // Overflow
            }
            value = newValue;
        } else if (str[i] == ' ' || str[i] == '\t') {
            // Skip trailing whitespace
            i++;
            while (str[i] && (str[i] == ' ' || str[i] == '\t')) {
                i++;
            }
            if (str[i]) return false;
            break;
        } else {
            return false;
        }
        i++;
    }

    result = static_cast<T>(value);
    return true;
}

} // anonymous namespace

// ============================================================================
// Int8 implementation
// ============================================================================

String Int8::ToString() const {
    return IntToString(static_cast<int>(_value));
}

Int8 Int8::Parse(const String& s) {
    Int8 result;
    if (!TryParse(s, result)) {
        throw FormatException("Input string was not in a correct format.");
    }
    return result;
}

bool Int8::TryParse(const String& s, Int8& result) {
    signed char val;
    if (TryParseInt<signed char>(s, val, MinValue, MaxValue)) {
        result = val;
        return true;
    }
    return false;
}

// ============================================================================
// UInt8 implementation
// ============================================================================

String UInt8::ToString() const {
    return UIntToString(static_cast<unsigned int>(_value));
}

UInt8 UInt8::Parse(const String& s) {
    UInt8 result;
    if (!TryParse(s, result)) {
        throw FormatException("Input string was not in a correct format.");
    }
    return result;
}

bool UInt8::TryParse(const String& s, UInt8& result) {
    unsigned char val;
    if (TryParseUInt<unsigned char>(s, val, MaxValue)) {
        result = val;
        return true;
    }
    return false;
}

// ============================================================================
// Int16 implementation
// ============================================================================

String Int16::ToString() const {
    return IntToString(static_cast<int>(_value));
}

Int16 Int16::Parse(const String& s) {
    Int16 result;
    if (!TryParse(s, result)) {
        throw FormatException("Input string was not in a correct format.");
    }
    return result;
}

bool Int16::TryParse(const String& s, Int16& result) {
    short val;
    if (TryParseInt<short>(s, val, MinValue, MaxValue)) {
        result = val;
        return true;
    }
    return false;
}

// ============================================================================
// UInt16 implementation
// ============================================================================

String UInt16::ToString() const {
    return UIntToString(static_cast<unsigned int>(_value));
}

UInt16 UInt16::Parse(const String& s) {
    UInt16 result;
    if (!TryParse(s, result)) {
        throw FormatException("Input string was not in a correct format.");
    }
    return result;
}

bool UInt16::TryParse(const String& s, UInt16& result) {
    unsigned short val;
    if (TryParseUInt<unsigned short>(s, val, MaxValue)) {
        result = val;
        return true;
    }
    return false;
}

// ============================================================================
// Int32 implementation
// ============================================================================

String Int32::ToString() const {
    return IntToString(_value);
}

Int32 Int32::Parse(const String& s) {
    Int32 result;
    if (!TryParse(s, result)) {
        throw FormatException("Input string was not in a correct format.");
    }
    return result;
}

bool Int32::TryParse(const String& s, Int32& result) {
    int val;
    if (TryParseInt<int>(s, val, MinValue, MaxValue)) {
        result = val;
        return true;
    }
    return false;
}

// ============================================================================
// UInt32 implementation
// ============================================================================

String UInt32::ToString() const {
    return UIntToString(_value);
}

UInt32 UInt32::Parse(const String& s) {
    UInt32 result;
    if (!TryParse(s, result)) {
        throw FormatException("Input string was not in a correct format.");
    }
    return result;
}

bool UInt32::TryParse(const String& s, UInt32& result) {
    unsigned int val;
    if (TryParseUInt<unsigned int>(s, val, MaxValue)) {
        result = val;
        return true;
    }
    return false;
}

// ============================================================================
// Int64 implementation
// ============================================================================

String Int64::ToString() const {
    return IntToString(_value);
}

Int64 Int64::Parse(const String& s) {
    Int64 result;
    if (!TryParse(s, result)) {
        throw FormatException("Input string was not in a correct format.");
    }
    return result;
}

bool Int64::TryParse(const String& s, Int64& result) {
    long long val;
    if (TryParseInt<long long>(s, val, MinValue, MaxValue)) {
        result = val;
        return true;
    }
    return false;
}

// ============================================================================
// UInt64 implementation
// ============================================================================

String UInt64::ToString() const {
    return UIntToString(_value);
}

UInt64 UInt64::Parse(const String& s) {
    UInt64 result;
    if (!TryParse(s, result)) {
        throw FormatException("Input string was not in a correct format.");
    }
    return result;
}

bool UInt64::TryParse(const String& s, UInt64& result) {
    unsigned long long val;
    if (TryParseUInt<unsigned long long>(s, val, MaxValue)) {
        result = val;
        return true;
    }
    return false;
}

// ============================================================================
// Float32 implementation
// ============================================================================

bool Float32::IsNaN(Float32 value) {
    return std::isnan(value._value);
}

bool Float32::IsInfinity(Float32 value) {
    return std::isinf(value._value);
}

bool Float32::IsPositiveInfinity(Float32 value) {
    return std::isinf(value._value) && value._value > 0;
}

bool Float32::IsNegativeInfinity(Float32 value) {
    return std::isinf(value._value) && value._value < 0;
}

String Float32::ToString() const {
    if (std::isnan(_value)) {
        return String("NaN");
    }
    if (std::isinf(_value)) {
        return _value > 0 ? String("Infinity") : String("-Infinity");
    }

    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%g", _value);
    return String(buffer);
}

Float32 Float32::Parse(const String& s) {
    Float32 result;
    if (!TryParse(s, result)) {
        throw FormatException("Input string was not in a correct format.");
    }
    return result;
}

bool Float32::TryParse(const String& s, Float32& result) {
    if (s.Length() == 0) {
        return false;
    }

    String trimmed = s.Trim();
    const char* str = trimmed.CStr();
    char* endptr;

    float value = std::strtof(str, &endptr);

    // Check if entire string was consumed
    if (*endptr != '\0') {
        return false;
    }

    result = value;
    return true;
}

int Float32::GetHashCode() const {
    // Simple bit cast for hash
    union {
        float f;
        int i;
    } u;
    u.f = _value;
    return u.i;
}

// ============================================================================
// Float64 implementation
// ============================================================================

bool Float64::IsNaN(Float64 value) {
    return std::isnan(value._value);
}

bool Float64::IsInfinity(Float64 value) {
    return std::isinf(value._value);
}

bool Float64::IsPositiveInfinity(Float64 value) {
    return std::isinf(value._value) && value._value > 0;
}

bool Float64::IsNegativeInfinity(Float64 value) {
    return std::isinf(value._value) && value._value < 0;
}

String Float64::ToString() const {
    if (std::isnan(_value)) {
        return String("NaN");
    }
    if (std::isinf(_value)) {
        return _value > 0 ? String("Infinity") : String("-Infinity");
    }

    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%g", _value);
    return String(buffer);
}

Float64 Float64::Parse(const String& s) {
    Float64 result;
    if (!TryParse(s, result)) {
        throw FormatException("Input string was not in a correct format.");
    }
    return result;
}

bool Float64::TryParse(const String& s, Float64& result) {
    if (s.Length() == 0) {
        return false;
    }

    String trimmed = s.Trim();
    const char* str = trimmed.CStr();
    char* endptr;

    double value = std::strtod(str, &endptr);

    // Check if entire string was consumed
    if (*endptr != '\0') {
        return false;
    }

    result = value;
    return true;
}

int Float64::GetHashCode() const {
    // Combine high and low bits for hash
    union {
        double d;
        long long ll;
    } u;
    u.d = _value;
    return static_cast<int>(u.ll ^ (u.ll >> 32));
}

} // namespace System
