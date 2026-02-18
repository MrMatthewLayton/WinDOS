#include "Types.hpp"  // Includes String.hpp and defines Int32, Boolean, Char
#include "Array.hpp"
#include "Exception.hpp"
#include "Math.hpp"
#include <cstring>
#include <cctype>

namespace System
{

// Static member initialization
const String String::Empty;

// ============================================================================
// Private helpers
// ============================================================================

void String::_copy(const char* src, Int32 len)
{
    if (len > 0 && src)
    {
        _length = len;
        _data = new char[static_cast<int>(_length) + 1];
        std::memcpy(_data, src, static_cast<int>(_length));
        _data[static_cast<int>(_length)] = '\0';
    }
    else
    {
        _data = nullptr;
        _length = Int32(0);
    }
}

void String::_free()
{
    delete[] _data;
    _data = nullptr;
    _length = Int32(0);
}

// ============================================================================
// Constructors
// ============================================================================

String::String() : _data(nullptr), _length(Int32(0))
{
}

String::String(const char* s) : _data(nullptr), _length(Int32(0))
{
    if (s)
    {
        _copy(s, Int32(std::strlen(s)));
    }
}

String::String(const char* s, Int32 length) : _data(nullptr), _length(Int32(0))
{
    if (s && length > 0)
    {
        _copy(s, length);
    }
}

String::String(Char c, Int32 count) : _data(nullptr), _length(Int32(0))
{
    if (count > 0)
    {
        _length = count;
        _data = new char[static_cast<int>(_length) + 1];
        std::memset(_data, static_cast<char>(c), static_cast<int>(_length));
        _data[static_cast<int>(_length)] = '\0';
    }
}

String::String(const String& other) : _data(nullptr), _length(Int32(0))
{
    _copy(other._data, other._length);
}

String::String(String&& other) noexcept : _data(other._data), _length(other._length)
{
    other._data = nullptr;
    other._length = Int32(0);
}

String::~String()
{
    _free();
}

// ============================================================================
// Assignment
// ============================================================================

String& String::operator=(const String& other)
{
    if (this != &other)
    {
        _free();
        _copy(other._data, other._length);
    }
    return *this;
}

String& String::operator=(String&& other) noexcept
{
    if (this != &other)
    {
        _free();
        _data = other._data;
        _length = other._length;
        other._data = nullptr;
        other._length = 0;
    }
    return *this;
}

String& String::operator=(const char* s)
{
    _free();
    if (s)
    {
        _copy(s, std::strlen(s));
    }
    return *this;
}

// ============================================================================
// Properties
// ============================================================================

Int32 String::Length() const
{
    return _length;
}

Boolean String::IsEmpty() const
{
    return Boolean(_length == 0);
}

Char String::operator[](Int32 index) const
{
    if (index < 0 || index >= _length)
    {
        throw IndexOutOfRangeException();
    }
    return Char(_data[static_cast<int>(index)]);
}

const char* String::GetRawString() const
{
    return _data ? _data : "";
}

// ============================================================================
// Substring
// ============================================================================

String String::Substring(Int32 startIndex) const
{
    return Substring(startIndex, _length - startIndex);
}

String String::Substring(Int32 startIndex, Int32 length) const
{
    if (startIndex < 0)
    {
        throw ArgumentOutOfRangeException("startIndex");
    }
    if (length < 0)
    {
        throw ArgumentOutOfRangeException("length");
    }
    if (startIndex > _length)
    {
        throw ArgumentOutOfRangeException("startIndex");
    }
    if (startIndex + length > _length)
    {
        throw ArgumentOutOfRangeException("length");
    }

    if (length == 0)
    {
        return String();
    }

    return String(_data + static_cast<int>(startIndex), length);
}

// ============================================================================
// Search
// ============================================================================

Int32 String::IndexOf(Char c) const
{
    return IndexOf(c, Int32(0));
}

Int32 String::IndexOf(Char c, Int32 startIndex) const
{
    Char ch = c;

    if (startIndex < 0 || startIndex > _length)
    {
        throw ArgumentOutOfRangeException("startIndex");
    }

    for (Int32 i = startIndex; i < _length; i += 1)
    {
        if (_data[static_cast<int>(i)] == static_cast<char>(ch))
        {
            return i;
        }
    }
    return Int32(-1);
}

Int32 String::IndexOf(const String& s) const
{
    return IndexOf(s, Int32(0));
}

Int32 String::IndexOf(const String& s, Int32 startIndex) const
{
    if (startIndex < 0 || startIndex > _length)
    {
        throw ArgumentOutOfRangeException("startIndex");
    }

    if (s._length == 0)
    {
        return startIndex;
    }

    if (s._length > _length - startIndex)
    {
        return Int32(-1);
    }

    for (Int32 i = startIndex; i <= _length - s._length; i += 1)
    {
        Boolean match = Boolean(true);
        for (Int32 j = Int32(0); j < s._length; j += 1)
        {
            if (_data[static_cast<int>(i + j)] != s._data[static_cast<int>(j)])
            {
                match = Boolean(false);
                break;
            }
        }
        if (match)
        {
            return i;
        }
    }
    return Int32(-1);
}

Int32 String::LastIndexOf(Char c) const
{
    Char ch = c;
    for (Int32 i = _length - 1; i >= 0; i -= 1)
    {
        if (_data[static_cast<int>(i)] == static_cast<char>(ch))
        {
            return i;
        }
    }
    return Int32(-1);
}

Int32 String::LastIndexOf(const String& s) const
{
    if (s._length == 0)
    {
        return _length;
    }

    if (s._length > _length)
    {
        return Int32(-1);
    }

    for (Int32 i = _length - s._length; i >= 0; i -= 1)
    {
        Boolean match = Boolean(true);
        for (Int32 j = Int32(0); j < s._length; j += 1)
        {
            if (_data[static_cast<int>(i + j)] != s._data[static_cast<int>(j)])
            {
                match = Boolean(false);
                break;
            }
        }
        if (match)
        {
            return i;
        }
    }
    return Int32(-1);
}

Boolean String::Contains(const String& s) const
{
    return Boolean(IndexOf(s) >= 0);
}

Boolean String::StartsWith(const String& s) const
{
    if (s._length > _length)
    {
        return Boolean(false);
    }
    if (s._length == 0)
    {
        return Boolean(true);
    }

    for (Int32 i = Int32(0); i < s._length; i += 1)
    {
        if (_data[static_cast<int>(i)] != s._data[static_cast<int>(i)])
        {
            return Boolean(false);
        }
    }
    return Boolean(true);
}

Boolean String::EndsWith(const String& s) const
{
    if (s._length > _length)
    {
        return Boolean(false);
    }
    if (s._length == 0)
    {
        return Boolean(true);
    }

    Int32 offset = _length - s._length;
    for (Int32 i = Int32(0); i < s._length; i += 1)
    {
        if (_data[static_cast<int>(offset + i)] != s._data[static_cast<int>(i)])
        {
            return Boolean(false);
        }
    }
    return Boolean(true);
}

// ============================================================================
// Transformation
// ============================================================================

String String::ToUpper() const
{
    if (_length == 0)
    {
        return String();
    }

    char* buffer = new char[static_cast<int>(_length) + 1];
    for (Int32 i = Int32(0); i < _length; i += 1)
    {
        buffer[static_cast<int>(i)] = std::toupper(static_cast<unsigned char>(_data[static_cast<int>(i)]));
    }
    buffer[static_cast<int>(_length)] = '\0';

    String result(buffer, _length);
    delete[] buffer;
    return result;
}

String String::ToLower() const
{
    if (_length == 0)
    {
        return String();
    }

    char* buffer = new char[static_cast<int>(_length) + 1];
    for (Int32 i = Int32(0); i < _length; i += 1)
    {
        buffer[static_cast<int>(i)] = std::tolower(static_cast<unsigned char>(_data[static_cast<int>(i)]));
    }
    buffer[static_cast<int>(_length)] = '\0';

    String result(buffer, _length);
    delete[] buffer;
    return result;
}

String String::Trim() const
{
    return TrimStart().TrimEnd();
}

String String::TrimStart() const
{
    if (_length == 0)
    {
        return String();
    }

    Int32 start = Int32(0);
    while (start < _length && std::isspace(static_cast<unsigned char>(_data[static_cast<int>(start)])))
    {
        start += 1;
    }

    if (start == _length)
    {
        return String();
    }

    return Substring(start);
}

String String::TrimEnd() const
{
    if (_length == 0)
    {
        return String();
    }

    Int32 end = _length;
    while (end > 0 && std::isspace(static_cast<unsigned char>(_data[static_cast<int>(end - 1)])))
    {
        end -= 1;
    }

    if (end == 0)
    {
        return String();
    }

    return Substring(Int32(0), end);
}

String String::Replace(Char oldChar, Char newChar) const
{
    if (_length == 0)
    {
        return String();
    }

    Char oldCh = oldChar;
    Char newCh = newChar;

    char* buffer = new char[static_cast<int>(_length) + 1];
    for (Int32 i = Int32(0); i < _length; i += 1)
    {
        buffer[static_cast<int>(i)] = (_data[static_cast<int>(i)] == static_cast<char>(oldCh)) ? static_cast<char>(newCh) : _data[static_cast<int>(i)];
    }
    buffer[static_cast<int>(_length)] = '\0';

    String result(buffer, _length);
    delete[] buffer;
    return result;
}

String String::Replace(const String& oldValue, const String& newValue) const
{
    if (oldValue._length == 0)
    {
        throw ArgumentException("String cannot be of zero length.", "oldValue");
    }

    if (_length == 0)
    {
        return String();
    }

    // Count occurrences
    Int32 count = Int32(0);
    Int32 pos = Int32(0);
    while ((pos = IndexOf(oldValue, pos)) >= 0)
    {
        count += 1;
        pos += oldValue._length;
    }

    if (count == 0)
    {
        return *this;
    }

    // Calculate new length with overflow protection
    Int32 lengthDiff = newValue._length - oldValue._length;
    Int32 totalDiff;
    if (!Math::TryMultiply(lengthDiff, count, totalDiff))
    {
        throw OverflowException("String replacement would result in overflow.");
    }
    Int32 checkedNewLength;
    if (!Math::TryAdd(_length, totalDiff, checkedNewLength))
    {
        throw OverflowException("String replacement would result in overflow.");
    }
    Int32 newLength = checkedNewLength;
    if (newLength < 0)
    {
        throw OverflowException("String replacement would result in negative length.");
    }
    char* buffer = new char[static_cast<int>(newLength) + 1];

    Int32 srcPos = Int32(0);
    Int32 dstPos = Int32(0);
    while (srcPos < _length)
    {
        Int32 foundPos = IndexOf(oldValue, srcPos);
        if (foundPos < 0)
        {
            // Copy rest
            std::memcpy(buffer + static_cast<int>(dstPos), _data + static_cast<int>(srcPos), static_cast<int>(_length - srcPos));
            dstPos += _length - srcPos;
            break;
        }

        // Copy before match
        if (foundPos > srcPos)
        {
            std::memcpy(buffer + static_cast<int>(dstPos), _data + static_cast<int>(srcPos), static_cast<int>(foundPos - srcPos));
            dstPos += foundPos - srcPos;
        }

        // Copy replacement
        if (newValue._length > 0)
        {
            std::memcpy(buffer + static_cast<int>(dstPos), newValue._data, static_cast<int>(newValue._length));
            dstPos += newValue._length;
        }

        srcPos = foundPos + oldValue._length;
    }

    buffer[static_cast<int>(newLength)] = '\0';
    String result(buffer, newLength);
    delete[] buffer;
    return result;
}

String String::Insert(Int32 startIndex, const String& value) const
{
    if (startIndex < 0 || startIndex > _length)
    {
        throw ArgumentOutOfRangeException("startIndex");
    }

    if (value._length == 0)
    {
        return *this;
    }

    Int32 newLength = _length + value._length;
    char* buffer = new char[static_cast<int>(newLength) + 1];

    if (startIndex > 0)
    {
        std::memcpy(buffer, _data, static_cast<int>(startIndex));
    }
    std::memcpy(buffer + static_cast<int>(startIndex), value._data, static_cast<int>(value._length));
    if (startIndex < _length)
    {
        std::memcpy(buffer + static_cast<int>(startIndex + value._length), _data + static_cast<int>(startIndex), static_cast<int>(_length - startIndex));
    }
    buffer[static_cast<int>(newLength)] = '\0';

    String result(buffer, newLength);
    delete[] buffer;
    return result;
}

String String::Remove(Int32 startIndex) const
{
    return Remove(startIndex, _length - startIndex);
}

String String::Remove(Int32 startIndex, Int32 count) const
{
    if (startIndex < 0)
    {
        throw ArgumentOutOfRangeException("startIndex");
    }
    if (count < 0)
    {
        throw ArgumentOutOfRangeException("count");
    }
    if (startIndex + count > _length)
    {
        throw ArgumentOutOfRangeException("count");
    }

    if (count == 0)
    {
        return *this;
    }

    Int32 newLength = _length - count;
    if (newLength == 0)
    {
        return String();
    }

    char* buffer = new char[static_cast<int>(newLength) + 1];
    if (startIndex > 0)
    {
        std::memcpy(buffer, _data, static_cast<int>(startIndex));
    }
    if (startIndex + count < _length)
    {
        std::memcpy(buffer + static_cast<int>(startIndex), _data + static_cast<int>(startIndex + count), static_cast<int>(_length - startIndex - count));
    }
    buffer[static_cast<int>(newLength)] = '\0';

    String result(buffer, newLength);
    delete[] buffer;
    return result;
}

String String::PadLeft(Int32 totalWidth) const
{
    return PadLeft(totalWidth, Char(' '));
}

String String::PadLeft(Int32 totalWidth, Char paddingChar) const
{
    if (totalWidth <= _length)
    {
        return *this;
    }

    Int32 padCount = totalWidth - _length;
    char* buffer = new char[static_cast<int>(totalWidth) + 1];
    std::memset(buffer, static_cast<char>(paddingChar), static_cast<int>(padCount));
    if (_length > 0)
    {
        std::memcpy(buffer + static_cast<int>(padCount), _data, static_cast<int>(_length));
    }
    buffer[static_cast<int>(totalWidth)] = '\0';

    String result(buffer, totalWidth);
    delete[] buffer;
    return result;
}

String String::PadRight(Int32 totalWidth) const
{
    return PadRight(totalWidth, Char(' '));
}

String String::PadRight(Int32 totalWidth, Char paddingChar) const
{
    if (totalWidth <= _length)
    {
        return *this;
    }

    char* buffer = new char[static_cast<int>(totalWidth) + 1];
    if (_length > 0)
    {
        std::memcpy(buffer, _data, static_cast<int>(_length));
    }
    std::memset(buffer + static_cast<int>(_length), static_cast<char>(paddingChar), static_cast<int>(totalWidth - _length));
    buffer[static_cast<int>(totalWidth)] = '\0';

    String result(buffer, totalWidth);
    delete[] buffer;
    return result;
}

// ============================================================================
// Split
// ============================================================================

Array<String> String::Split(Char delimiter) const
{
    char delims[2] = { static_cast<char>(delimiter), '\0' };
    return Split(delims);
}

Array<String> String::Split(const char* delimiters) const
{
    if (_length == 0 || !delimiters || delimiters[0] == '\0')
    {
        Array<String> result(1);
        result[Int32(0)] = *this;
        return result;
    }

    // Count parts
    Int32 count = Int32(1);
    for (Int32 i = Int32(0); i < _length; i += 1)
    {
        for (const char* d = delimiters; *d; d++)
        {
            if (_data[static_cast<int>(i)] == *d)
            {
                count += 1;
                break;
            }
        }
    }

    Array<String> result(count);
    Int32 partIndex = Int32(0);
    Int32 start = Int32(0);

    for (Int32 i = Int32(0); i <= _length; i += 1)
    {
        Boolean isDelimiter = Boolean(false);
        if (i < _length)
        {
            for (const char* d = delimiters; *d; d++)
            {
                if (_data[static_cast<int>(i)] == *d)
                {
                    isDelimiter = Boolean(true);
                    break;
                }
            }
        }

        if (isDelimiter || i == _length)
        {
            result[partIndex] = Substring(start, i - start);
            partIndex += 1;
            start = i + 1;
        }
    }

    return result;
}

// ============================================================================
// Comparison operators
// ============================================================================

Boolean String::operator==(const String& other) const
{
    if (_length != other._length)
    {
        return Boolean(false);
    }
    if (_length == 0)
    {
        return Boolean(true);
    }
    return Boolean(std::memcmp(_data, other._data, static_cast<int>(_length)) == 0);
}

Boolean String::operator!=(const String& other) const
{
    return Boolean(!(*this == other));
}

Boolean String::operator<(const String& other) const
{
    return Boolean(CompareTo(other) < 0);
}

Boolean String::operator>(const String& other) const
{
    return Boolean(CompareTo(other) > 0);
}

Boolean String::operator<=(const String& other) const
{
    return Boolean(CompareTo(other) <= 0);
}

Boolean String::operator>=(const String& other) const
{
    return Boolean(CompareTo(other) >= 0);
}

Boolean String::operator==(const char* other) const
{
    if (!other)
    {
        return Boolean(_length == 0);
    }
    return *this == String(other);
}

Boolean String::operator!=(const char* other) const
{
    return Boolean(!(*this == other));
}

// ============================================================================
// Concatenation
// ============================================================================

String String::operator+(const String& other) const
{
    if (other._length == 0)
    {
        return *this;
    }
    if (_length == 0)
    {
        return other;
    }

    // Check for overflow
    Int32 newLengthChecked;
    if (!Math::TryAdd(_length, other._length, newLengthChecked))
    {
        throw OverflowException("String concatenation would result in overflow.");
    }
    Int32 newLength = newLengthChecked;
    char* buffer = new char[static_cast<int>(newLength) + 1];
    std::memcpy(buffer, _data, static_cast<int>(_length));
    std::memcpy(buffer + static_cast<int>(_length), other._data, static_cast<int>(other._length));
    buffer[static_cast<int>(newLength)] = '\0';

    String result(buffer, newLength);
    delete[] buffer;
    return result;
}

String String::operator+(const char* other) const
{
    if (!other || other[0] == '\0')
    {
        return *this;
    }
    return *this + String(other);
}

String String::operator+(Char c) const
{
    Int32 newLength = _length + 1;
    char* buffer = new char[static_cast<int>(newLength) + 1];
    if (_length > 0)
    {
        std::memcpy(buffer, _data, static_cast<int>(_length));
    }
    buffer[static_cast<int>(_length)] = static_cast<char>(c);
    buffer[static_cast<int>(newLength)] = '\0';

    String result(buffer, newLength);
    delete[] buffer;
    return result;
}

String& String::operator+=(const String& other)
{
    *this = *this + other;
    return *this;
}

String& String::operator+=(const char* other)
{
    *this = *this + other;
    return *this;
}

String& String::operator+=(Char c)
{
    *this = *this + c;
    return *this;
}

// ============================================================================
// Static methods
// ============================================================================

Boolean String::IsNullOrEmpty(const String& s)
{
    return Boolean(s._length == 0);
}

Boolean String::IsNullOrWhiteSpace(const String& s)
{
    if (s._length == 0)
    {
        return Boolean(true);
    }
    for (Int32 i = Int32(0); i < s._length; i += 1)
    {
        if (!std::isspace(static_cast<unsigned char>(s._data[static_cast<int>(i)])))
        {
            return Boolean(false);
        }
    }
    return Boolean(true);
}

String String::Concat(const String& s1, const String& s2)
{
    return s1 + s2;
}

String String::Concat(const String& s1, const String& s2, const String& s3)
{
    return s1 + s2 + s3;
}

Int32 String::Compare(const String& s1, const String& s2)
{
    return s1.CompareTo(s2);
}

Int32 String::CompareIgnoreCase(const String& s1, const String& s2)
{
    Int32 len = (s1._length < s2._length) ? s1._length : s2._length;

    for (Int32 i = Int32(0); i < len; i += 1)
    {
        Int32 c1 = Int32(std::tolower(static_cast<unsigned char>(s1._data[static_cast<int>(i)])));
        Int32 c2 = Int32(std::tolower(static_cast<unsigned char>(s2._data[static_cast<int>(i)])));
        if (c1 != c2)
        {
            return c1 - c2;
        }
    }

    return s1._length - s2._length;
}

// ============================================================================
// Comparison methods
// ============================================================================

Int32 String::CompareTo(const String& other) const
{
    if (_data == nullptr && other._data == nullptr)
    {
        return Int32(0);
    }
    if (_data == nullptr)
    {
        return Int32(-1);
    }
    if (other._data == nullptr)
    {
        return Int32(1);
    }

    Int32 len = (_length < other._length) ? _length : other._length;
    Int32 cmp = Int32(std::memcmp(_data, other._data, static_cast<int>(len)));
    if (cmp != 0)
    {
        return cmp;
    }
    return _length - other._length;
}

Boolean String::Equals(const String& other) const
{
    return *this == other;
}

Boolean String::EqualsIgnoreCase(const String& other) const
{
    return Boolean(CompareIgnoreCase(*this, other) == 0);
}

Int32 String::GetHashCode() const
{
    if (_length == 0)
    {
        return Int32(0);
    }

    // Simple FNV-1a hash
    UInt32 hash = UInt32(2166136261u);
    for (Int32 i = Int32(0); i < _length; i += 1)
    {
        hash ^= UInt32(static_cast<unsigned char>(_data[static_cast<int>(i)]));
        hash *= UInt32(16777619u);
    }
    return Int32(static_cast<int>(hash));
}

// ============================================================================
// Non-member operators
// ============================================================================

String operator+(const char* lhs, const String& rhs)
{
    return String(lhs) + rhs;
}

// ============================================================================
// StringBuilder Implementation
// ============================================================================

const Int32 StringBuilder::DEFAULT_CAPACITY = Int32(16);
const Int32 StringBuilder::GROWTH_FACTOR = Int32(2);

void StringBuilder::EnsureCapacity(Int32 minCapacity)
{
    if (minCapacity <= _capacity)
    {
        return;
    }

    // Grow by at least GROWTH_FACTOR or to minCapacity, whichever is larger
    Int32 newCapacity = _capacity * GROWTH_FACTOR;
    if (newCapacity < minCapacity)
    {
        newCapacity = minCapacity;
    }

    char* newBuffer = new char[static_cast<int>(newCapacity)];
    if (_buffer && _length > 0)
    {
        std::memcpy(newBuffer, _buffer, static_cast<int>(_length));
    }
    newBuffer[static_cast<int>(_length)] = '\0';

    delete[] _buffer;
    _buffer = newBuffer;
    _capacity = newCapacity;
}

StringBuilder::StringBuilder()
    : _buffer(nullptr)
    , _length(Int32(0))
    , _capacity(Int32(0))
{
    EnsureCapacity(DEFAULT_CAPACITY);
}

StringBuilder::StringBuilder(Int32 capacity)
    : _buffer(nullptr)
    , _length(Int32(0))
    , _capacity(Int32(0))
{
    Int32 cap = capacity;
    if (cap < DEFAULT_CAPACITY)
    {
        cap = DEFAULT_CAPACITY;
    }
    EnsureCapacity(cap);
}

StringBuilder::StringBuilder(const String& value)
    : _buffer(nullptr)
    , _length(Int32(0))
    , _capacity(Int32(0))
{
    Int32 len = value.Length();
    EnsureCapacity(len + DEFAULT_CAPACITY);
    if (len > 0)
    {
        std::memcpy(_buffer, value.GetRawString(), static_cast<int>(len));
        _length = len;
        _buffer[static_cast<int>(_length)] = '\0';
    }
}

StringBuilder::StringBuilder(const StringBuilder& other)
    : _buffer(nullptr)
    , _length(Int32(0))
    , _capacity(Int32(0))
{
    EnsureCapacity(other._capacity);
    if (other._length > 0)
    {
        std::memcpy(_buffer, other._buffer, static_cast<int>(other._length));
        _length = other._length;
        _buffer[static_cast<int>(_length)] = '\0';
    }
}

StringBuilder::StringBuilder(StringBuilder&& other) noexcept
    : _buffer(other._buffer)
    , _length(other._length)
    , _capacity(other._capacity)
{
    other._buffer = nullptr;
    other._length = Int32(0);
    other._capacity = Int32(0);
}

StringBuilder::~StringBuilder()
{
    delete[] _buffer;
}

StringBuilder& StringBuilder::operator=(const StringBuilder& other)
{
    if (this != &other)
    {
        EnsureCapacity(other._length + 1);
        if (other._length > 0)
        {
            std::memcpy(_buffer, other._buffer, static_cast<int>(other._length));
        }
        _length = other._length;
        _buffer[static_cast<int>(_length)] = '\0';
    }
    return *this;
}

StringBuilder& StringBuilder::operator=(StringBuilder&& other) noexcept
{
    if (this != &other)
    {
        delete[] _buffer;
        _buffer = other._buffer;
        _length = other._length;
        _capacity = other._capacity;
        other._buffer = nullptr;
        other._length = Int32(0);
        other._capacity = Int32(0);
    }
    return *this;
}

Int32 StringBuilder::Length() const
{
    return _length;
}

Int32 StringBuilder::Capacity() const
{
    return _capacity;
}

Char StringBuilder::operator[](Int32 index) const
{
    if (index < 0 || index >= _length)
    {
        throw IndexOutOfRangeException();
    }
    return Char(_buffer[static_cast<int>(index)]);
}

void StringBuilder::SetCharAt(Int32 index, Char c)
{
    if (index < 0 || index >= _length)
    {
        throw IndexOutOfRangeException();
    }
    _buffer[static_cast<int>(index)] = static_cast<char>(c);
}

StringBuilder& StringBuilder::Append(const String& value)
{
    Int32 len = value.Length();
    if (len > 0)
    {
        EnsureCapacity(_length + len + 1);
        std::memcpy(_buffer + static_cast<int>(_length), value.GetRawString(), static_cast<int>(len));
        _length += len;
        _buffer[static_cast<int>(_length)] = '\0';
    }
    return *this;
}

StringBuilder& StringBuilder::Append(const char* value)
{
    if (value)
    {
        Int32 len = Int32(std::strlen(value));
        if (len > 0)
        {
            EnsureCapacity(_length + len + 1);
            std::memcpy(_buffer + static_cast<int>(_length), value, static_cast<int>(len));
            _length += len;
            _buffer[static_cast<int>(_length)] = '\0';
        }
    }
    return *this;
}

StringBuilder& StringBuilder::Append(Char value)
{
    EnsureCapacity(_length + 2);
    _buffer[static_cast<int>(_length)] = static_cast<char>(value);
    _length += 1;
    _buffer[static_cast<int>(_length)] = '\0';
    return *this;
}

StringBuilder& StringBuilder::Append(char value)
{
    EnsureCapacity(_length + 2);
    _buffer[static_cast<int>(_length)] = value;
    _length += 1;
    _buffer[static_cast<int>(_length)] = '\0';
    return *this;
}

StringBuilder& StringBuilder::Append(Int32 value)
{
    // Convert integer to string
    char temp[12];  // Enough for -2147483648
    Int32 val = value;
    Boolean negative = val < 0;
    if (negative)
    {
        val = -val;
    }

    Int32 i = Int32(0);
    do
    {
        temp[static_cast<int>(i)] = '0' + static_cast<int>(val % 10);
        i += 1;
        val /= 10;
    }
    while (val > 0);

    if (negative)
    {
        temp[static_cast<int>(i)] = '-';
        i += 1;
    }

    // Reverse and append
    EnsureCapacity(_length + i + 1);
    while (i > 0)
    {
        i -= 1;
        _buffer[static_cast<int>(_length)] = temp[static_cast<int>(i)];
        _length += 1;
    }
    _buffer[static_cast<int>(_length)] = '\0';
    return *this;
}

StringBuilder& StringBuilder::Append(Boolean value)
{
    return Append(static_cast<bool>(value) ? "True" : "False");
}

StringBuilder& StringBuilder::AppendLine()
{
    return Append('\n');
}

StringBuilder& StringBuilder::AppendLine(const String& value)
{
    Append(value);
    return Append('\n');
}

StringBuilder& StringBuilder::AppendLine(const char* value)
{
    Append(value);
    return Append('\n');
}

StringBuilder& StringBuilder::Insert(Int32 index, const String& value)
{
    if (index < 0 || index > _length)
    {
        throw ArgumentOutOfRangeException("index");
    }

    Int32 len = value.Length();
    if (len > 0)
    {
        EnsureCapacity(_length + len + 1);
        // Shift existing characters to make room
        std::memmove(_buffer + static_cast<int>(index) + static_cast<int>(len),
                     _buffer + static_cast<int>(index),
                     static_cast<int>(_length - index));
        std::memcpy(_buffer + static_cast<int>(index), value.GetRawString(), static_cast<int>(len));
        _length += len;
        _buffer[static_cast<int>(_length)] = '\0';
    }
    return *this;
}

StringBuilder& StringBuilder::Insert(Int32 index, const char* value)
{
    return Insert(index, String(value));
}

StringBuilder& StringBuilder::Insert(Int32 index, Char value)
{
    if (index < 0 || index > _length)
    {
        throw ArgumentOutOfRangeException("index");
    }

    EnsureCapacity(_length + 2);
    std::memmove(_buffer + static_cast<int>(index) + 1,
                 _buffer + static_cast<int>(index),
                 static_cast<int>(_length - index));
    _buffer[static_cast<int>(index)] = static_cast<char>(value);
    _length += 1;
    _buffer[static_cast<int>(_length)] = '\0';
    return *this;
}

StringBuilder& StringBuilder::Remove(Int32 startIndex, Int32 length)
{
    if (startIndex < 0 || startIndex >= _length)
    {
        throw ArgumentOutOfRangeException("startIndex");
    }
    if (length < 0 || startIndex + length > _length)
    {
        throw ArgumentOutOfRangeException("length");
    }

    if (length > 0)
    {
        std::memmove(_buffer + static_cast<int>(startIndex),
                     _buffer + static_cast<int>(startIndex + length),
                     static_cast<int>(_length - startIndex - length));
        _length -= length;
        _buffer[static_cast<int>(_length)] = '\0';
    }
    return *this;
}

StringBuilder& StringBuilder::Clear()
{
    _length = Int32(0);
    if (_buffer)
    {
        _buffer[0] = '\0';
    }
    return *this;
}

void StringBuilder::Reserve(Int32 capacity)
{
    EnsureCapacity(capacity);
}

String StringBuilder::ToString() const
{
    if (_length == 0)
    {
        return String::Empty;
    }
    return String(_buffer, _length);
}

} // namespace System
