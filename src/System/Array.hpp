#ifndef SYSTEM_ARRAY_HPP
#define SYSTEM_ARRAY_HPP

#include "Exception.hpp"
#include <initializer_list>
#include <utility>

namespace System
{

/// @brief A generic array class that provides bounds-checked element access and common array operations.
/// @tparam T The type of elements stored in the array.
///
/// Array<T> is a managed array container similar to .NET's System.Array. It provides:
/// - Automatic memory management (RAII)
/// - Bounds-checked element access
/// - Copy and move semantics
/// - Initializer list support
/// - Range-based for loop compatibility
///
/// Example usage:
/// @code
/// Array<int> numbers(5);           // Create array of 5 integers
/// numbers[0] = 42;                 // Set element
/// int value = numbers[0];          // Get element
///
/// Array<String> names = {"Alice", "Bob", "Charlie"};  // Initializer list
/// for (const auto& name : names) { ... }              // Range-based for
/// @endcode
template<typename T>
class Array
{
    T* _data;
    int _length;

    /// @brief Allocates memory for the array.
    /// @param length The number of elements to allocate.
    void _allocate(int length)
    {
        if (length > 0)
        {
            _length = length;
            _data = new T[_length]();  // Value-initialize
        }
        else
        {
            _data = nullptr;
            _length = 0;
        }
    }

    /// @brief Frees the array memory and resets to empty state.
    void _free()
    {
        delete[] _data;
        _data = nullptr;
        _length = 0;
    }

    /// @brief Copies elements from a source array.
    /// @param src Pointer to source elements.
    /// @param length Number of elements to copy.
    void _copyFrom(const T* src, int length)
    {
        _allocate(length);
        for (int i = 0; i < _length; i++)
        {
            _data[i] = src[i];
        }
    }

public:
    /// @brief Constructs an empty array with zero elements.
    Array() : _data(nullptr), _length(0)
    {
    }

    /// @brief Constructs an array with the specified number of value-initialized elements.
    /// @param length The number of elements in the array.
    /// @throws ArgumentOutOfRangeException if length is negative.
    explicit Array(int length) : _data(nullptr), _length(0)
    {
        if (length < 0)
        {
            throw ArgumentOutOfRangeException("length", "Non-negative number required.");
        }
        _allocate(length);
    }

    /// @brief Constructs an array from an initializer list.
    /// @param init The initializer list containing elements to copy into the array.
    ///
    /// Example:
    /// @code
    /// Array<int> numbers = {1, 2, 3, 4, 5};
    /// @endcode
    Array(std::initializer_list<T> init) : _data(nullptr), _length(0)
    {
        _allocate(static_cast<int>(init.size()));
        int i = 0;
        for (const auto& item : init)
        {
            _data[i++] = item;
        }
    }

    /// @brief Copy constructor. Creates a deep copy of another array.
    /// @param other The array to copy from.
    Array(const Array& other) : _data(nullptr), _length(0)
    {
        if (other._length > 0)
        {
            _copyFrom(other._data, other._length);
        }
    }

    /// @brief Move constructor. Transfers ownership from another array.
    /// @param other The array to move from. Will be left in an empty state.
    Array(Array&& other) noexcept : _data(other._data), _length(other._length)
    {
        other._data = nullptr;
        other._length = 0;
    }

    /// @brief Destructor. Frees all allocated memory.
    ~Array()
    {
        _free();
    }

    /// @brief Copy assignment operator. Replaces contents with a deep copy of another array.
    /// @param other The array to copy from.
    /// @return Reference to this array.
    Array& operator=(const Array& other)
    {
        if (this != &other)
        {
            _free();
            if (other._length > 0)
            {
                _copyFrom(other._data, other._length);
            }
        }
        return *this;
    }

    /// @brief Move assignment operator. Transfers ownership from another array.
    /// @param other The array to move from. Will be left in an empty state.
    /// @return Reference to this array.
    Array& operator=(Array&& other) noexcept
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

    /// @brief Initializer list assignment operator. Replaces contents with elements from the list.
    /// @param init The initializer list containing elements to copy into the array.
    /// @return Reference to this array.
    ///
    /// Example:
    /// @code
    /// Array<int> numbers;
    /// numbers = {10, 20, 30};
    /// @endcode
    Array& operator=(std::initializer_list<T> init)
    {
        _free();
        _allocate(static_cast<int>(init.size()));
        int i = 0;
        for (const auto& item : init)
        {
            _data[i++] = item;
        }
        return *this;
    }

    /// @brief Gets the number of elements in the array.
    /// @return The total number of elements.
    int Length() const
    {
        return _length;
    }

    /// @brief Checks whether the array contains no elements.
    /// @return true if the array has zero elements; otherwise, false.
    bool IsEmpty() const
    {
        return _length == 0;
    }

    /// @brief Accesses the element at the specified index with bounds checking.
    /// @param index The zero-based index of the element to access.
    /// @return Reference to the element at the specified index.
    /// @throws IndexOutOfRangeException if index is less than 0 or greater than or equal to Length().
    T& operator[](int index)
    {
        if (index < 0 || index >= _length)
        {
            throw IndexOutOfRangeException();
        }
        return _data[index];
    }

    /// @brief Accesses the element at the specified index with bounds checking (const version).
    /// @param index The zero-based index of the element to access.
    /// @return Const reference to the element at the specified index.
    /// @throws IndexOutOfRangeException if index is less than 0 or greater than or equal to Length().
    const T& operator[](int index) const
    {
        if (index < 0 || index >= _length)
        {
            throw IndexOutOfRangeException();
        }
        return _data[index];
    }

    /// @brief Gets the value at the specified index.
    /// @param index The zero-based index of the element to get.
    /// @return Reference to the element at the specified index.
    /// @throws IndexOutOfRangeException if index is out of bounds.
    ///
    /// This method is equivalent to operator[] but provides a named alternative.
    T& GetValue(int index)
    {
        return (*this)[index];
    }

    /// @brief Gets the value at the specified index (const version).
    /// @param index The zero-based index of the element to get.
    /// @return Const reference to the element at the specified index.
    /// @throws IndexOutOfRangeException if index is out of bounds.
    const T& GetValue(int index) const
    {
        return (*this)[index];
    }

    /// @brief Sets the value at the specified index.
    /// @param index The zero-based index of the element to set.
    /// @param value The value to assign to the element.
    /// @throws IndexOutOfRangeException if index is out of bounds.
    void SetValue(int index, const T& value)
    {
        (*this)[index] = value;
    }

    /// @brief Returns an iterator to the beginning of the array.
    /// @return Pointer to the first element, or nullptr if the array is empty.
    ///
    /// Enables range-based for loop support:
    /// @code
    /// Array<int> arr = {1, 2, 3};
    /// for (int& x : arr) { x *= 2; }
    /// @endcode
    T* begin()
    {
        return _data;
    }

    /// @brief Returns an iterator to the end of the array.
    /// @return Pointer to one past the last element.
    T* end()
    {
        return _data + _length;
    }

    /// @brief Returns a const iterator to the beginning of the array.
    /// @return Const pointer to the first element, or nullptr if the array is empty.
    const T* begin() const
    {
        return _data;
    }

    /// @brief Returns a const iterator to the end of the array.
    /// @return Const pointer to one past the last element.
    const T* end() const
    {
        return _data + _length;
    }

    /// @brief Gets direct access to the underlying data buffer.
    /// @return Pointer to the first element, or nullptr if the array is empty.
    ///
    /// @warning Use with caution. The returned pointer is only valid as long as
    /// the array exists and is not resized. Modifying elements through this pointer
    /// bypasses bounds checking.
    T* Data()
    {
        return _data;
    }

    /// @brief Gets direct read-only access to the underlying data buffer.
    /// @return Const pointer to the first element, or nullptr if the array is empty.
    const T* Data() const
    {
        return _data;
    }

    /// @brief Resets all elements in the array to their default value.
    ///
    /// Each element is assigned T(), which is the default-constructed value
    /// (0 for numeric types, nullptr for pointers, etc.).
    /// The array length remains unchanged.
    void Clear()
    {
        for (int i = 0; i < _length; i++)
        {
            _data[i] = T();
        }
    }

    /// @brief Changes the number of elements in the array.
    /// @param newLength The new number of elements.
    /// @throws ArgumentOutOfRangeException if newLength is negative.
    ///
    /// If newLength is greater than the current length, new elements are
    /// value-initialized. If newLength is smaller, excess elements are discarded.
    /// Existing elements within the new bounds are preserved via move semantics.
    ///
    /// Example:
    /// @code
    /// Array<int> arr = {1, 2, 3};
    /// arr.Resize(5);  // arr is now {1, 2, 3, 0, 0}
    /// arr.Resize(2);  // arr is now {1, 2}
    /// @endcode
    void Resize(int newLength)
    {
        if (newLength < 0)
        {
            throw ArgumentOutOfRangeException("newLength", "Non-negative number required.");
        }

        if (newLength == _length)
        {
            return;
        }

        if (newLength == 0)
        {
            _free();
            return;
        }

        T* newData = new T[newLength]();
        int copyCount = (_length < newLength) ? _length : newLength;
        for (int i = 0; i < copyCount; i++)
        {
            newData[i] = std::move(_data[i]);
        }

        delete[] _data;
        _data = newData;
        _length = newLength;
    }

    /// @brief Copies all elements to another array starting at the specified index.
    /// @param destination The destination array to copy elements to.
    /// @param destinationIndex The zero-based index in the destination at which copying begins.
    /// @throws ArgumentOutOfRangeException if destinationIndex is negative.
    /// @throws ArgumentException if the destination array is not large enough to hold all copied elements.
    ///
    /// Example:
    /// @code
    /// Array<int> source = {1, 2, 3};
    /// Array<int> dest(5);
    /// source.CopyTo(dest, 1);  // dest is now {0, 1, 2, 3, 0}
    /// @endcode
    void CopyTo(Array<T>& destination, int destinationIndex) const
    {
        if (destinationIndex < 0)
        {
            throw ArgumentOutOfRangeException("destinationIndex");
        }
        if (destinationIndex + _length > destination._length)
        {
            throw ArgumentException("Destination array is not long enough.");
        }

        for (int i = 0; i < _length; i++)
        {
            destination._data[destinationIndex + i] = _data[i];
        }
    }

    /// @brief Reverses the order of elements in the array in place.
    ///
    /// Uses move semantics for efficient element swapping.
    ///
    /// Example:
    /// @code
    /// Array<int> arr = {1, 2, 3, 4, 5};
    /// arr.Reverse();  // arr is now {5, 4, 3, 2, 1}
    /// @endcode
    void Reverse()
    {
        for (int i = 0; i < _length / 2; i++)
        {
            T temp = std::move(_data[i]);
            _data[i] = std::move(_data[_length - 1 - i]);
            _data[_length - 1 - i] = std::move(temp);
        }
    }

    /// @brief Searches for the specified value and returns the index of its first occurrence.
    /// @param value The value to locate in the array.
    /// @return The zero-based index of the first occurrence of value, or -1 if not found.
    ///
    /// Uses operator== for element comparison. Performs a linear search from index 0.
    ///
    /// Example:
    /// @code
    /// Array<int> arr = {10, 20, 30, 20};
    /// int idx = arr.IndexOf(20);  // Returns 1 (first occurrence)
    /// int notFound = arr.IndexOf(99);  // Returns -1
    /// @endcode
    int IndexOf(const T& value) const
    {
        for (int i = 0; i < _length; i++)
        {
            if (_data[i] == value)
            {
                return i;
            }
        }
        return -1;
    }

    /// @brief Determines whether the array contains a specific value.
    /// @param value The value to locate in the array.
    /// @return true if the array contains the value; otherwise, false.
    ///
    /// Uses operator== for element comparison. Equivalent to IndexOf(value) >= 0.
    bool Contains(const T& value) const
    {
        return IndexOf(value) >= 0;
    }

    /// @brief Creates an array from a raw pointer and length.
    /// @param data Pointer to the source elements to copy.
    /// @param length The number of elements to copy.
    /// @return A new Array containing copies of the source elements.
    /// @throws ArgumentOutOfRangeException if length is negative.
    ///
    /// This is a static factory method for creating arrays from C-style arrays or buffers.
    ///
    /// Example:
    /// @code
    /// int rawData[] = {1, 2, 3, 4, 5};
    /// Array<int> arr = Array<int>::FromPointer(rawData, 5);
    /// @endcode
    static Array<T> FromPointer(const T* data, int length)
    {
        if (length < 0)
        {
            throw ArgumentOutOfRangeException("length");
        }
        if (length == 0)
        {
            return Array<T>();
        }

        Array<T> result(length);
        for (int i = 0; i < length; i++)
        {
            result._data[i] = data[i];
        }
        return result;
    }
};

} // namespace System

#endif // SYSTEM_ARRAY_HPP
