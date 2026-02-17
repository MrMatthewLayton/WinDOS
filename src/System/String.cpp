#include "Types.hpp"  // Includes String.hpp and defines Int32, Boolean, Char
#include "Array.hpp"
#include "Exception.hpp"
#include "Math.hpp"
#include <cstring>
#include <cctype>

namespace System {

// Static member initialization
const String String::Empty;

// ============================================================================
// Private helpers
// ============================================================================

void String::_copy(const char* src, int len) {
    if (len > 0 && src) {
        _length = len;
        _data = new char[_length + 1];
        std::memcpy(_data, src, _length);
        _data[_length] = '\0';
    } else {
        _data = nullptr;
        _length = 0;
    }
}

void String::_free() {
    delete[] _data;
    _data = nullptr;
    _length = 0;
}

// ============================================================================
// Constructors
// ============================================================================

String::String() : _data(nullptr), _length(0) {
}

String::String(const char* s) : _data(nullptr), _length(0) {
    if (s) {
        _copy(s, std::strlen(s));
    }
}

String::String(const char* s, Int32 length) : _data(nullptr), _length(0) {
    int len = static_cast<int>(length);
    if (s && len > 0) {
        _copy(s, len);
    }
}

String::String(Char c, Int32 count) : _data(nullptr), _length(0) {
    int cnt = static_cast<int>(count);
    if (cnt > 0) {
        _length = cnt;
        _data = new char[_length + 1];
        std::memset(_data, static_cast<char>(c), _length);
        _data[_length] = '\0';
    }
}

String::String(const String& other) : _data(nullptr), _length(0) {
    _copy(other._data, other._length);
}

String::String(String&& other) noexcept : _data(other._data), _length(other._length) {
    other._data = nullptr;
    other._length = 0;
}

String::~String() {
    _free();
}

// ============================================================================
// Assignment
// ============================================================================

String& String::operator=(const String& other) {
    if (this != &other) {
        _free();
        _copy(other._data, other._length);
    }
    return *this;
}

String& String::operator=(String&& other) noexcept {
    if (this != &other) {
        _free();
        _data = other._data;
        _length = other._length;
        other._data = nullptr;
        other._length = 0;
    }
    return *this;
}

String& String::operator=(const char* s) {
    _free();
    if (s) {
        _copy(s, std::strlen(s));
    }
    return *this;
}

// ============================================================================
// Properties
// ============================================================================

Int32 String::Length() const {
    return Int32(_length);
}

Boolean String::IsEmpty() const {
    return Boolean(_length == 0);
}

Char String::operator[](Int32 index) const {
    int idx = static_cast<int>(index);
    if (idx < 0 || idx >= _length) {
        throw IndexOutOfRangeException();
    }
    return Char(_data[idx]);
}

const char* String::CStr() const {
    return _data ? _data : "";
}

// ============================================================================
// Substring
// ============================================================================

String String::Substring(Int32 startIndex) const {
    int start = static_cast<int>(startIndex);
    return Substring(start, _length - start);
}

String String::Substring(Int32 startIndex, Int32 length) const {
    int start = static_cast<int>(startIndex);
    int len = static_cast<int>(length);

    if (start < 0) {
        throw ArgumentOutOfRangeException("startIndex");
    }
    if (len < 0) {
        throw ArgumentOutOfRangeException("length");
    }
    if (start > _length) {
        throw ArgumentOutOfRangeException("startIndex");
    }
    if (start + len > _length) {
        throw ArgumentOutOfRangeException("length");
    }

    if (len == 0) {
        return String();
    }

    return String(_data + start, len);
}

// ============================================================================
// Search
// ============================================================================

Int32 String::IndexOf(Char c) const {
    return IndexOf(c, Int32(0));
}

Int32 String::IndexOf(Char c, Int32 startIndex) const {
    int start = static_cast<int>(startIndex);
    char ch = static_cast<char>(c);

    if (start < 0 || start > _length) {
        throw ArgumentOutOfRangeException("startIndex");
    }

    for (int i = start; i < _length; i++) {
        if (_data[i] == ch) {
            return Int32(i);
        }
    }
    return Int32(-1);
}

Int32 String::IndexOf(const String& s) const {
    return IndexOf(s, Int32(0));
}

Int32 String::IndexOf(const String& s, Int32 startIndex) const {
    int start = static_cast<int>(startIndex);

    if (start < 0 || start > _length) {
        throw ArgumentOutOfRangeException("startIndex");
    }

    if (s._length == 0) {
        return Int32(start);
    }

    if (s._length > _length - start) {
        return Int32(-1);
    }

    for (int i = start; i <= _length - s._length; i++) {
        bool match = true;
        for (int j = 0; j < s._length; j++) {
            if (_data[i + j] != s._data[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            return Int32(i);
        }
    }
    return Int32(-1);
}

Int32 String::LastIndexOf(Char c) const {
    char ch = static_cast<char>(c);
    for (int i = _length - 1; i >= 0; i--) {
        if (_data[i] == ch) {
            return Int32(i);
        }
    }
    return Int32(-1);
}

Int32 String::LastIndexOf(const String& s) const {
    if (s._length == 0) {
        return Int32(_length);
    }

    if (s._length > _length) {
        return Int32(-1);
    }

    for (int i = _length - s._length; i >= 0; i--) {
        bool match = true;
        for (int j = 0; j < s._length; j++) {
            if (_data[i + j] != s._data[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            return Int32(i);
        }
    }
    return Int32(-1);
}

Boolean String::Contains(const String& s) const {
    return Boolean(static_cast<int>(IndexOf(s)) >= 0);
}

Boolean String::StartsWith(const String& s) const {
    if (s._length > _length) {
        return Boolean(false);
    }
    if (s._length == 0) {
        return Boolean(true);
    }

    for (int i = 0; i < s._length; i++) {
        if (_data[i] != s._data[i]) {
            return Boolean(false);
        }
    }
    return Boolean(true);
}

Boolean String::EndsWith(const String& s) const {
    if (s._length > _length) {
        return Boolean(false);
    }
    if (s._length == 0) {
        return Boolean(true);
    }

    int offset = _length - s._length;
    for (int i = 0; i < s._length; i++) {
        if (_data[offset + i] != s._data[i]) {
            return Boolean(false);
        }
    }
    return Boolean(true);
}

// ============================================================================
// Transformation
// ============================================================================

String String::ToUpper() const {
    if (_length == 0) {
        return String();
    }

    char* buffer = new char[_length + 1];
    for (int i = 0; i < _length; i++) {
        buffer[i] = std::toupper(static_cast<unsigned char>(_data[i]));
    }
    buffer[_length] = '\0';

    String result(buffer, _length);
    delete[] buffer;
    return result;
}

String String::ToLower() const {
    if (_length == 0) {
        return String();
    }

    char* buffer = new char[_length + 1];
    for (int i = 0; i < _length; i++) {
        buffer[i] = std::tolower(static_cast<unsigned char>(_data[i]));
    }
    buffer[_length] = '\0';

    String result(buffer, _length);
    delete[] buffer;
    return result;
}

String String::Trim() const {
    return TrimStart().TrimEnd();
}

String String::TrimStart() const {
    if (_length == 0) {
        return String();
    }

    int start = 0;
    while (start < _length && std::isspace(static_cast<unsigned char>(_data[start]))) {
        start++;
    }

    if (start == _length) {
        return String();
    }

    return Substring(start);
}

String String::TrimEnd() const {
    if (_length == 0) {
        return String();
    }

    int end = _length;
    while (end > 0 && std::isspace(static_cast<unsigned char>(_data[end - 1]))) {
        end--;
    }

    if (end == 0) {
        return String();
    }

    return Substring(0, end);
}

String String::Replace(Char oldChar, Char newChar) const {
    if (_length == 0) {
        return String();
    }

    char oldCh = static_cast<char>(oldChar);
    char newCh = static_cast<char>(newChar);

    char* buffer = new char[_length + 1];
    for (int i = 0; i < _length; i++) {
        buffer[i] = (_data[i] == oldCh) ? newCh : _data[i];
    }
    buffer[_length] = '\0';

    String result(buffer, _length);
    delete[] buffer;
    return result;
}

String String::Replace(const String& oldValue, const String& newValue) const {
    if (oldValue._length == 0) {
        throw ArgumentException("String cannot be of zero length.", "oldValue");
    }

    if (_length == 0) {
        return String();
    }

    // Count occurrences
    int count = 0;
    int pos = 0;
    while ((pos = static_cast<int>(IndexOf(oldValue, pos))) >= 0) {
        count++;
        pos += oldValue._length;
    }

    if (count == 0) {
        return *this;
    }

    // Calculate new length with overflow protection
    int lengthDiff = newValue._length - oldValue._length;
    Int32 totalDiff;
    if (!Math::TryMultiply(Int32(lengthDiff), Int32(count), totalDiff)) {
        throw OverflowException("String replacement would result in overflow.");
    }
    Int32 checkedNewLength;
    if (!Math::TryAdd(Int32(_length), totalDiff, checkedNewLength)) {
        throw OverflowException("String replacement would result in overflow.");
    }
    int newLength = static_cast<int>(checkedNewLength);
    if (newLength < 0) {
        throw OverflowException("String replacement would result in negative length.");
    }
    char* buffer = new char[newLength + 1];

    int srcPos = 0;
    int dstPos = 0;
    while (srcPos < _length) {
        int foundPos = static_cast<int>(IndexOf(oldValue, srcPos));
        if (foundPos < 0) {
            // Copy rest
            std::memcpy(buffer + dstPos, _data + srcPos, _length - srcPos);
            dstPos += _length - srcPos;
            break;
        }

        // Copy before match
        if (foundPos > srcPos) {
            std::memcpy(buffer + dstPos, _data + srcPos, foundPos - srcPos);
            dstPos += foundPos - srcPos;
        }

        // Copy replacement
        if (newValue._length > 0) {
            std::memcpy(buffer + dstPos, newValue._data, newValue._length);
            dstPos += newValue._length;
        }

        srcPos = foundPos + oldValue._length;
    }

    buffer[newLength] = '\0';
    String result(buffer, newLength);
    delete[] buffer;
    return result;
}

String String::Insert(Int32 startIndex, const String& value) const {
    int start = static_cast<int>(startIndex);

    if (start < 0 || start > _length) {
        throw ArgumentOutOfRangeException("startIndex");
    }

    if (value._length == 0) {
        return *this;
    }

    int newLength = _length + value._length;
    char* buffer = new char[newLength + 1];

    if (start > 0) {
        std::memcpy(buffer, _data, start);
    }
    std::memcpy(buffer + start, value._data, value._length);
    if (start < _length) {
        std::memcpy(buffer + start + value._length, _data + start, _length - start);
    }
    buffer[newLength] = '\0';

    String result(buffer, newLength);
    delete[] buffer;
    return result;
}

String String::Remove(Int32 startIndex) const {
    int start = static_cast<int>(startIndex);
    return Remove(start, _length - start);
}

String String::Remove(Int32 startIndex, Int32 count) const {
    int start = static_cast<int>(startIndex);
    int cnt = static_cast<int>(count);

    if (start < 0) {
        throw ArgumentOutOfRangeException("startIndex");
    }
    if (cnt < 0) {
        throw ArgumentOutOfRangeException("count");
    }
    if (start + cnt > _length) {
        throw ArgumentOutOfRangeException("count");
    }

    if (cnt == 0) {
        return *this;
    }

    int newLength = _length - cnt;
    if (newLength == 0) {
        return String();
    }

    char* buffer = new char[newLength + 1];
    if (start > 0) {
        std::memcpy(buffer, _data, start);
    }
    if (start + cnt < _length) {
        std::memcpy(buffer + start, _data + start + cnt, _length - start - cnt);
    }
    buffer[newLength] = '\0';

    String result(buffer, newLength);
    delete[] buffer;
    return result;
}

String String::PadLeft(Int32 totalWidth, char paddingChar) const {
    int total = static_cast<int>(totalWidth);

    if (total <= _length) {
        return *this;
    }

    int padCount = total - _length;
    char* buffer = new char[total + 1];
    std::memset(buffer, paddingChar, padCount);
    if (_length > 0) {
        std::memcpy(buffer + padCount, _data, _length);
    }
    buffer[total] = '\0';

    String result(buffer, total);
    delete[] buffer;
    return result;
}

String String::PadRight(Int32 totalWidth, char paddingChar) const {
    int total = static_cast<int>(totalWidth);

    if (total <= _length) {
        return *this;
    }

    char* buffer = new char[total + 1];
    if (_length > 0) {
        std::memcpy(buffer, _data, _length);
    }
    std::memset(buffer + _length, paddingChar, total - _length);
    buffer[total] = '\0';

    String result(buffer, total);
    delete[] buffer;
    return result;
}

// ============================================================================
// Split
// ============================================================================

Array<String> String::Split(Char delimiter) const {
    char delims[2] = { static_cast<char>(delimiter), '\0' };
    return Split(delims);
}

Array<String> String::Split(const char* delimiters) const {
    if (_length == 0 || !delimiters || delimiters[0] == '\0') {
        Array<String> result(1);
        result[0] = *this;
        return result;
    }

    // Count parts
    int count = 1;
    for (int i = 0; i < _length; i++) {
        for (const char* d = delimiters; *d; d++) {
            if (_data[i] == *d) {
                count++;
                break;
            }
        }
    }

    Array<String> result(count);
    int partIndex = 0;
    int start = 0;

    for (int i = 0; i <= _length; i++) {
        bool isDelimiter = false;
        if (i < _length) {
            for (const char* d = delimiters; *d; d++) {
                if (_data[i] == *d) {
                    isDelimiter = true;
                    break;
                }
            }
        }

        if (isDelimiter || i == _length) {
            result[partIndex++] = Substring(start, i - start);
            start = i + 1;
        }
    }

    return result;
}

// ============================================================================
// Comparison operators
// ============================================================================

bool String::operator==(const String& other) const {
    if (_length != other._length) {
        return false;
    }
    if (_length == 0) {
        return true;
    }
    return std::memcmp(_data, other._data, _length) == 0;
}

bool String::operator!=(const String& other) const {
    return !(*this == other);
}

bool String::operator<(const String& other) const {
    return static_cast<int>(CompareTo(other)) < 0;
}

bool String::operator>(const String& other) const {
    return static_cast<int>(CompareTo(other)) > 0;
}

bool String::operator<=(const String& other) const {
    return static_cast<int>(CompareTo(other)) <= 0;
}

bool String::operator>=(const String& other) const {
    return static_cast<int>(CompareTo(other)) >= 0;
}

bool String::operator==(const char* other) const {
    if (!other) {
        return _length == 0;
    }
    return *this == String(other);
}

bool String::operator!=(const char* other) const {
    return !(*this == other);
}

// ============================================================================
// Concatenation
// ============================================================================

String String::operator+(const String& other) const {
    if (other._length == 0) {
        return *this;
    }
    if (_length == 0) {
        return other;
    }

    // Check for overflow
    Int32 newLengthChecked;
    if (!Math::TryAdd(Int32(_length), Int32(other._length), newLengthChecked)) {
        throw OverflowException("String concatenation would result in overflow.");
    }
    int newLength = static_cast<int>(newLengthChecked);
    char* buffer = new char[newLength + 1];
    std::memcpy(buffer, _data, _length);
    std::memcpy(buffer + _length, other._data, other._length);
    buffer[newLength] = '\0';

    String result(buffer, newLength);
    delete[] buffer;
    return result;
}

String String::operator+(const char* other) const {
    if (!other || other[0] == '\0') {
        return *this;
    }
    return *this + String(other);
}

String String::operator+(Char c) const {
    int newLength = _length + 1;
    char* buffer = new char[newLength + 1];
    if (_length > 0) {
        std::memcpy(buffer, _data, _length);
    }
    buffer[_length] = static_cast<char>(c);
    buffer[newLength] = '\0';

    String result(buffer, newLength);
    delete[] buffer;
    return result;
}

String& String::operator+=(const String& other) {
    *this = *this + other;
    return *this;
}

String& String::operator+=(const char* other) {
    *this = *this + other;
    return *this;
}

String& String::operator+=(Char c) {
    *this = *this + c;
    return *this;
}

// ============================================================================
// Static methods
// ============================================================================

Boolean String::IsNullOrEmpty(const String& s) {
    return Boolean(s._length == 0);
}

Boolean String::IsNullOrWhiteSpace(const String& s) {
    if (s._length == 0) {
        return Boolean(true);
    }
    for (int i = 0; i < s._length; i++) {
        if (!std::isspace(static_cast<unsigned char>(s._data[i]))) {
            return Boolean(false);
        }
    }
    return Boolean(true);
}

String String::Concat(const String& s1, const String& s2) {
    return s1 + s2;
}

String String::Concat(const String& s1, const String& s2, const String& s3) {
    return s1 + s2 + s3;
}

Int32 String::Compare(const String& s1, const String& s2) {
    return s1.CompareTo(s2);
}

Int32 String::CompareIgnoreCase(const String& s1, const String& s2) {
    int len = (s1._length < s2._length) ? s1._length : s2._length;

    for (int i = 0; i < len; i++) {
        int c1 = std::tolower(static_cast<unsigned char>(s1._data[i]));
        int c2 = std::tolower(static_cast<unsigned char>(s2._data[i]));
        if (c1 != c2) {
            return Int32(c1 - c2);
        }
    }

    return Int32(s1._length - s2._length);
}

// ============================================================================
// Comparison methods
// ============================================================================

Int32 String::CompareTo(const String& other) const {
    if (_data == nullptr && other._data == nullptr) {
        return Int32(0);
    }
    if (_data == nullptr) {
        return Int32(-1);
    }
    if (other._data == nullptr) {
        return Int32(1);
    }

    int len = (_length < other._length) ? _length : other._length;
    int cmp = std::memcmp(_data, other._data, len);
    if (cmp != 0) {
        return Int32(cmp);
    }
    return Int32(_length - other._length);
}

Boolean String::Equals(const String& other) const {
    return Boolean(*this == other);
}

Boolean String::EqualsIgnoreCase(const String& other) const {
    return Boolean(static_cast<int>(CompareIgnoreCase(*this, other)) == 0);
}

Int32 String::GetHashCode() const {
    if (_length == 0) {
        return Int32(0);
    }

    // Simple FNV-1a hash
    unsigned int hash = 2166136261u;
    for (int i = 0; i < _length; i++) {
        hash ^= static_cast<unsigned char>(_data[i]);
        hash *= 16777619u;
    }
    return Int32(static_cast<int>(hash));
}

// ============================================================================
// Non-member operators
// ============================================================================

String operator+(const char* lhs, const String& rhs) {
    return String(lhs) + rhs;
}

// ============================================================================
// StringBuilder Implementation
// ============================================================================

void StringBuilder::EnsureCapacity(int minCapacity) {
    if (minCapacity <= _capacity) return;

    // Grow by at least GROWTH_FACTOR or to minCapacity, whichever is larger
    int newCapacity = _capacity * GROWTH_FACTOR;
    if (newCapacity < minCapacity) {
        newCapacity = minCapacity;
    }

    char* newBuffer = new char[newCapacity];
    if (_buffer && _length > 0) {
        std::memcpy(newBuffer, _buffer, _length);
    }
    newBuffer[_length] = '\0';

    delete[] _buffer;
    _buffer = newBuffer;
    _capacity = newCapacity;
}

StringBuilder::StringBuilder()
    : _buffer(nullptr)
    , _length(0)
    , _capacity(0) {
    EnsureCapacity(DEFAULT_CAPACITY);
}

StringBuilder::StringBuilder(Int32 capacity)
    : _buffer(nullptr)
    , _length(0)
    , _capacity(0) {
    int cap = static_cast<int>(capacity);
    if (cap < DEFAULT_CAPACITY) cap = DEFAULT_CAPACITY;
    EnsureCapacity(cap);
}

StringBuilder::StringBuilder(const String& value)
    : _buffer(nullptr)
    , _length(0)
    , _capacity(0) {
    int len = static_cast<int>(value.Length());
    EnsureCapacity(len + DEFAULT_CAPACITY);
    if (len > 0) {
        std::memcpy(_buffer, value.CStr(), len);
        _length = len;
        _buffer[_length] = '\0';
    }
}

StringBuilder::StringBuilder(const StringBuilder& other)
    : _buffer(nullptr)
    , _length(0)
    , _capacity(0) {
    EnsureCapacity(other._capacity);
    if (other._length > 0) {
        std::memcpy(_buffer, other._buffer, other._length);
        _length = other._length;
        _buffer[_length] = '\0';
    }
}

StringBuilder::StringBuilder(StringBuilder&& other) noexcept
    : _buffer(other._buffer)
    , _length(other._length)
    , _capacity(other._capacity) {
    other._buffer = nullptr;
    other._length = 0;
    other._capacity = 0;
}

StringBuilder::~StringBuilder() {
    delete[] _buffer;
}

StringBuilder& StringBuilder::operator=(const StringBuilder& other) {
    if (this != &other) {
        EnsureCapacity(other._length + 1);
        if (other._length > 0) {
            std::memcpy(_buffer, other._buffer, other._length);
        }
        _length = other._length;
        _buffer[_length] = '\0';
    }
    return *this;
}

StringBuilder& StringBuilder::operator=(StringBuilder&& other) noexcept {
    if (this != &other) {
        delete[] _buffer;
        _buffer = other._buffer;
        _length = other._length;
        _capacity = other._capacity;
        other._buffer = nullptr;
        other._length = 0;
        other._capacity = 0;
    }
    return *this;
}

Int32 StringBuilder::Length() const {
    return Int32(_length);
}

Int32 StringBuilder::Capacity() const {
    return Int32(_capacity);
}

Char StringBuilder::operator[](Int32 index) const {
    int idx = static_cast<int>(index);
    if (idx < 0 || idx >= _length) {
        throw IndexOutOfRangeException();
    }
    return Char(_buffer[idx]);
}

void StringBuilder::SetCharAt(Int32 index, Char c) {
    int idx = static_cast<int>(index);
    if (idx < 0 || idx >= _length) {
        throw IndexOutOfRangeException();
    }
    _buffer[idx] = static_cast<char>(c);
}

StringBuilder& StringBuilder::Append(const String& value) {
    int len = static_cast<int>(value.Length());
    if (len > 0) {
        EnsureCapacity(_length + len + 1);
        std::memcpy(_buffer + _length, value.CStr(), len);
        _length += len;
        _buffer[_length] = '\0';
    }
    return *this;
}

StringBuilder& StringBuilder::Append(const char* value) {
    if (value) {
        int len = static_cast<int>(std::strlen(value));
        if (len > 0) {
            EnsureCapacity(_length + len + 1);
            std::memcpy(_buffer + _length, value, len);
            _length += len;
            _buffer[_length] = '\0';
        }
    }
    return *this;
}

StringBuilder& StringBuilder::Append(Char value) {
    EnsureCapacity(_length + 2);
    _buffer[_length++] = static_cast<char>(value);
    _buffer[_length] = '\0';
    return *this;
}

StringBuilder& StringBuilder::Append(char value) {
    EnsureCapacity(_length + 2);
    _buffer[_length++] = value;
    _buffer[_length] = '\0';
    return *this;
}

StringBuilder& StringBuilder::Append(Int32 value) {
    // Convert integer to string
    char temp[12];  // Enough for -2147483648
    int val = static_cast<int>(value);
    bool negative = val < 0;
    if (negative) val = -val;

    int i = 0;
    do {
        temp[i++] = '0' + (val % 10);
        val /= 10;
    } while (val > 0);

    if (negative) {
        temp[i++] = '-';
    }

    // Reverse and append
    EnsureCapacity(_length + i + 1);
    while (i > 0) {
        _buffer[_length++] = temp[--i];
    }
    _buffer[_length] = '\0';
    return *this;
}

StringBuilder& StringBuilder::Append(bool value) {
    return Append(value ? "True" : "False");
}

StringBuilder& StringBuilder::AppendLine() {
    return Append('\n');
}

StringBuilder& StringBuilder::AppendLine(const String& value) {
    Append(value);
    return Append('\n');
}

StringBuilder& StringBuilder::AppendLine(const char* value) {
    Append(value);
    return Append('\n');
}

StringBuilder& StringBuilder::Insert(Int32 index, const String& value) {
    int idx = static_cast<int>(index);
    if (idx < 0 || idx > _length) {
        throw ArgumentOutOfRangeException("index");
    }

    int len = static_cast<int>(value.Length());
    if (len > 0) {
        EnsureCapacity(_length + len + 1);
        // Shift existing characters to make room
        std::memmove(_buffer + idx + len, _buffer + idx, _length - idx);
        std::memcpy(_buffer + idx, value.CStr(), len);
        _length += len;
        _buffer[_length] = '\0';
    }
    return *this;
}

StringBuilder& StringBuilder::Insert(Int32 index, const char* value) {
    return Insert(index, String(value));
}

StringBuilder& StringBuilder::Insert(Int32 index, Char value) {
    int idx = static_cast<int>(index);
    if (idx < 0 || idx > _length) {
        throw ArgumentOutOfRangeException("index");
    }

    EnsureCapacity(_length + 2);
    std::memmove(_buffer + idx + 1, _buffer + idx, _length - idx);
    _buffer[idx] = static_cast<char>(value);
    _length++;
    _buffer[_length] = '\0';
    return *this;
}

StringBuilder& StringBuilder::Remove(Int32 startIndex, Int32 length) {
    int start = static_cast<int>(startIndex);
    int len = static_cast<int>(length);

    if (start < 0 || start >= _length) {
        throw ArgumentOutOfRangeException("startIndex");
    }
    if (len < 0 || start + len > _length) {
        throw ArgumentOutOfRangeException("length");
    }

    if (len > 0) {
        std::memmove(_buffer + start, _buffer + start + len, _length - start - len);
        _length -= len;
        _buffer[_length] = '\0';
    }
    return *this;
}

StringBuilder& StringBuilder::Clear() {
    _length = 0;
    if (_buffer) {
        _buffer[0] = '\0';
    }
    return *this;
}

void StringBuilder::Reserve(Int32 capacity) {
    EnsureCapacity(static_cast<int>(capacity));
}

String StringBuilder::ToString() const {
    if (_length == 0) {
        return String::Empty;
    }
    return String(_buffer, Int32(_length));
}

} // namespace System
