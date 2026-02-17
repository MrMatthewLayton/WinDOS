#ifndef SYSTEM_ARRAY_HPP
#define SYSTEM_ARRAY_HPP

#include "Exception.hpp"
#include <initializer_list>
#include <utility>

namespace System {

template<typename T>
class Array {
private:
    T* _data;
    int _length;

    void _allocate(int length) {
        if (length > 0) {
            _length = length;
            _data = new T[_length]();  // Value-initialize
        } else {
            _data = nullptr;
            _length = 0;
        }
    }

    void _free() {
        delete[] _data;
        _data = nullptr;
        _length = 0;
    }

    void _copyFrom(const T* src, int length) {
        _allocate(length);
        for (int i = 0; i < _length; i++) {
            _data[i] = src[i];
        }
    }

public:
    // Default constructor - empty array
    Array() : _data(nullptr), _length(0) {
    }

    // Constructor with length
    explicit Array(int length) : _data(nullptr), _length(0) {
        if (length < 0) {
            throw ArgumentOutOfRangeException("length", "Non-negative number required.");
        }
        _allocate(length);
    }

    // Initializer list constructor
    Array(std::initializer_list<T> init) : _data(nullptr), _length(0) {
        _allocate(static_cast<int>(init.size()));
        int i = 0;
        for (const auto& item : init) {
            _data[i++] = item;
        }
    }

    // Copy constructor
    Array(const Array& other) : _data(nullptr), _length(0) {
        if (other._length > 0) {
            _copyFrom(other._data, other._length);
        }
    }

    // Move constructor
    Array(Array&& other) noexcept : _data(other._data), _length(other._length) {
        other._data = nullptr;
        other._length = 0;
    }

    // Destructor
    ~Array() {
        _free();
    }

    // Copy assignment
    Array& operator=(const Array& other) {
        if (this != &other) {
            _free();
            if (other._length > 0) {
                _copyFrom(other._data, other._length);
            }
        }
        return *this;
    }

    // Move assignment
    Array& operator=(Array&& other) noexcept {
        if (this != &other) {
            _free();
            _data = other._data;
            _length = other._length;
            other._data = nullptr;
            other._length = 0;
        }
        return *this;
    }

    // Initializer list assignment
    Array& operator=(std::initializer_list<T> init) {
        _free();
        _allocate(static_cast<int>(init.size()));
        int i = 0;
        for (const auto& item : init) {
            _data[i++] = item;
        }
        return *this;
    }

    // Length property
    int Length() const {
        return _length;
    }

    // Check if empty
    bool IsEmpty() const {
        return _length == 0;
    }

    // Element access with bounds checking
    T& operator[](int index) {
        if (index < 0 || index >= _length) {
            throw IndexOutOfRangeException();
        }
        return _data[index];
    }

    const T& operator[](int index) const {
        if (index < 0 || index >= _length) {
            throw IndexOutOfRangeException();
        }
        return _data[index];
    }

    // Get element (same as operator[], but named method)
    T& GetValue(int index) {
        return (*this)[index];
    }

    const T& GetValue(int index) const {
        return (*this)[index];
    }

    // Set element
    void SetValue(int index, const T& value) {
        (*this)[index] = value;
    }

    // Iterator support for range-based for loops
    T* begin() {
        return _data;
    }

    T* end() {
        return _data + _length;
    }

    const T* begin() const {
        return _data;
    }

    const T* end() const {
        return _data + _length;
    }

    // Get raw pointer (use with caution)
    T* Data() {
        return _data;
    }

    const T* Data() const {
        return _data;
    }

    // Clear the array (set all to default)
    void Clear() {
        for (int i = 0; i < _length; i++) {
            _data[i] = T();
        }
    }

    // Resize the array (creates new array, copies elements)
    void Resize(int newLength) {
        if (newLength < 0) {
            throw ArgumentOutOfRangeException("newLength", "Non-negative number required.");
        }

        if (newLength == _length) {
            return;
        }

        if (newLength == 0) {
            _free();
            return;
        }

        T* newData = new T[newLength]();
        int copyCount = (_length < newLength) ? _length : newLength;
        for (int i = 0; i < copyCount; i++) {
            newData[i] = std::move(_data[i]);
        }

        delete[] _data;
        _data = newData;
        _length = newLength;
    }

    // Copy to another array
    void CopyTo(Array<T>& destination, int destinationIndex) const {
        if (destinationIndex < 0) {
            throw ArgumentOutOfRangeException("destinationIndex");
        }
        if (destinationIndex + _length > destination._length) {
            throw ArgumentException("Destination array is not long enough.");
        }

        for (int i = 0; i < _length; i++) {
            destination._data[destinationIndex + i] = _data[i];
        }
    }

    // Reverse the array in place
    void Reverse() {
        for (int i = 0; i < _length / 2; i++) {
            T temp = std::move(_data[i]);
            _data[i] = std::move(_data[_length - 1 - i]);
            _data[_length - 1 - i] = std::move(temp);
        }
    }

    // Find index of element (-1 if not found)
    int IndexOf(const T& value) const {
        for (int i = 0; i < _length; i++) {
            if (_data[i] == value) {
                return i;
            }
        }
        return -1;
    }

    // Check if array contains element
    bool Contains(const T& value) const {
        return IndexOf(value) >= 0;
    }

    // Static method to create array from raw pointer
    static Array<T> FromPointer(const T* data, int length) {
        if (length < 0) {
            throw ArgumentOutOfRangeException("length");
        }
        if (length == 0) {
            return Array<T>();
        }

        Array<T> result(length);
        for (int i = 0; i < length; i++) {
            result._data[i] = data[i];
        }
        return result;
    }
};

} // namespace System

#endif // SYSTEM_ARRAY_HPP
