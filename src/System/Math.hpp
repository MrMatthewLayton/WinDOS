#ifndef SYSTEM_MATH_HPP
#define SYSTEM_MATH_HPP

#include "Types.hpp"
#include "Exception.hpp"
#include <cstddef>  // for size_t

namespace System
{

/// @brief Provides constants and static methods for common mathematical functions
///        and checked arithmetic operations.
///
/// The Math class is a static utility class that provides basic mathematical
/// operations (Abs, Min, Max, Clamp, Swap) as well as checked arithmetic
/// operations that detect integer overflow and underflow conditions.
///
/// Checked operations (CheckedAdd, CheckedSubtract, CheckedMultiply) throw
/// OverflowException when overflow would occur, while Try operations
/// (TryAdd, TrySubtract, TryMultiply) return false instead of throwing.
///
/// This class cannot be instantiated.
class Math
{
public:
    // ========================================================================
    // Basic Operations
    // ========================================================================

    /// @brief Returns the absolute value of a 32-bit signed integer.
    /// @param value The integer value to compute the absolute value of.
    /// @return The absolute value of the input. If value is negative, returns -value;
    ///         otherwise returns value unchanged.
    static Int32 Abs(Int32 value)
    {
        int v = static_cast<int>(value);
        return Int32(v < 0 ? -v : v);
    }

    /// @brief Returns the smaller of two 32-bit signed integers.
    /// @param a The first value to compare.
    /// @param b The second value to compare.
    /// @return The smaller of the two values. If values are equal, returns that value.
    static Int32 Min(Int32 a, Int32 b)
    {
        int va = static_cast<int>(a);
        int vb = static_cast<int>(b);
        return Int32(va < vb ? va : vb);
    }

    /// @brief Returns the larger of two 32-bit signed integers.
    /// @param a The first value to compare.
    /// @param b The second value to compare.
    /// @return The larger of the two values. If values are equal, returns that value.
    static Int32 Max(Int32 a, Int32 b)
    {
        int va = static_cast<int>(a);
        int vb = static_cast<int>(b);
        return Int32(va > vb ? va : vb);
    }

    /// @brief Clamps a value to be within a specified range.
    /// @param value The value to clamp.
    /// @param min The minimum allowed value (inclusive).
    /// @param max The maximum allowed value (inclusive).
    /// @return If value < min, returns min. If value > max, returns max.
    ///         Otherwise returns value unchanged.
    static Int32 Clamp(Int32 value, Int32 min, Int32 max)
    {
        int v = static_cast<int>(value);
        int vmin = static_cast<int>(min);
        int vmax = static_cast<int>(max);
        if (v < vmin)
        {
            return Int32(vmin);
        }
        if (v > vmax)
        {
            return Int32(vmax);
        }
        return Int32(v);
    }

    /// @brief Swaps the values of two 32-bit signed integer references.
    /// @param a Reference to the first value, which will receive b's original value.
    /// @param b Reference to the second value, which will receive a's original value.
    static void Swap(Int32& a, Int32& b)
    {
        Int32 temp = a;
        a = b;
        b = temp;
    }

    // ========================================================================
    // Checked Arithmetic - throws OverflowException on overflow
    // ========================================================================

    /// @brief Adds two Int32 values with overflow checking.
    /// @param a The first addend.
    /// @param b The second addend.
    /// @return The sum of a and b.
    /// @throws OverflowException If the result would overflow Int32::MaxValue
    ///         (positive overflow) or underflow Int32::MinValue (negative overflow).
    static Int32 CheckedAdd(Int32 a, Int32 b)
    {
        int va = static_cast<int>(a);
        int vb = static_cast<int>(b);

        // Check for overflow
        if (vb > 0 && va > Int32::MaxValue - vb)
        {
            throw OverflowException("Integer overflow in addition.");
        }
        if (vb < 0 && va < Int32::MinValue - vb)
        {
            throw OverflowException("Integer underflow in addition.");
        }

        return Int32(va + vb);
    }

    /// @brief Subtracts two Int32 values with overflow checking.
    /// @param a The minuend (value to subtract from).
    /// @param b The subtrahend (value to subtract).
    /// @return The difference (a - b).
    /// @throws OverflowException If the result would overflow Int32::MaxValue
    ///         (positive overflow) or underflow Int32::MinValue (negative overflow).
    static Int32 CheckedSubtract(Int32 a, Int32 b)
    {
        int va = static_cast<int>(a);
        int vb = static_cast<int>(b);

        // Check for overflow
        if (vb < 0 && va > Int32::MaxValue + vb)
        {
            throw OverflowException("Integer overflow in subtraction.");
        }
        if (vb > 0 && va < Int32::MinValue + vb)
        {
            throw OverflowException("Integer underflow in subtraction.");
        }

        return Int32(va - vb);
    }

    /// @brief Multiplies two Int32 values with overflow checking.
    /// @param a The first factor (multiplicand).
    /// @param b The second factor (multiplier).
    /// @return The product of a and b.
    /// @throws OverflowException If the result would overflow Int32::MaxValue
    ///         or underflow Int32::MinValue.
    static Int32 CheckedMultiply(Int32 a, Int32 b)
    {
        int va = static_cast<int>(a);
        int vb = static_cast<int>(b);

        if (va == 0 || vb == 0)
        {
            return Int32(0);
        }

        // Check for overflow using 64-bit arithmetic
        long long result = static_cast<long long>(va) * static_cast<long long>(vb);
        if (result > Int32::MaxValue || result < Int32::MinValue)
        {
            throw OverflowException("Integer overflow in multiplication.");
        }

        return Int32(static_cast<int>(result));
    }

    /// @brief Casts a size_t value to Int32 with overflow checking.
    /// @param value The size_t value to convert.
    /// @return The value as an Int32.
    /// @throws OverflowException If value exceeds Int32::MaxValue.
    static Int32 CheckedCast(size_t value)
    {
        if (value > static_cast<size_t>(Int32::MaxValue))
        {
            throw OverflowException("Value too large to fit in Int32.");
        }
        return Int32(static_cast<int>(value));
    }

    /// @brief Casts a long long value to Int32 with overflow checking.
    /// @param value The long long value to convert.
    /// @return The value as an Int32.
    /// @throws OverflowException If value is less than Int32::MinValue or
    ///         greater than Int32::MaxValue.
    static Int32 CheckedCast(long long value)
    {
        if (value > Int32::MaxValue || value < Int32::MinValue)
        {
            throw OverflowException("Value out of range for Int32.");
        }
        return Int32(static_cast<int>(value));
    }

    // ========================================================================
    // Safe arithmetic - returns success/failure instead of throwing
    // ========================================================================

    /// @brief Attempts to add two Int32 values without throwing on overflow.
    /// @param a The first addend.
    /// @param b The second addend.
    /// @param result Output parameter that receives the sum if successful.
    ///               Unchanged if overflow would occur.
    /// @return True if the addition succeeded without overflow; false if
    ///         overflow would have occurred.
    static bool TryAdd(Int32 a, Int32 b, Int32& result)
    {
        int va = static_cast<int>(a);
        int vb = static_cast<int>(b);

        if (vb > 0 && va > Int32::MaxValue - vb)
        {
            return false;
        }
        if (vb < 0 && va < Int32::MinValue - vb)
        {
            return false;
        }

        result = Int32(va + vb);
        return true;
    }

    /// @brief Attempts to subtract two Int32 values without throwing on overflow.
    /// @param a The minuend (value to subtract from).
    /// @param b The subtrahend (value to subtract).
    /// @param result Output parameter that receives the difference if successful.
    ///               Unchanged if overflow would occur.
    /// @return True if the subtraction succeeded without overflow; false if
    ///         overflow would have occurred.
    static bool TrySubtract(Int32 a, Int32 b, Int32& result)
    {
        int va = static_cast<int>(a);
        int vb = static_cast<int>(b);

        if (vb < 0 && va > Int32::MaxValue + vb)
        {
            return false;
        }
        if (vb > 0 && va < Int32::MinValue + vb)
        {
            return false;
        }

        result = Int32(va - vb);
        return true;
    }

    /// @brief Attempts to multiply two Int32 values without throwing on overflow.
    /// @param a The first factor (multiplicand).
    /// @param b The second factor (multiplier).
    /// @param result Output parameter that receives the product if successful.
    ///               Set to 0 if either operand is 0, unchanged otherwise if
    ///               overflow would occur.
    /// @return True if the multiplication succeeded without overflow; false if
    ///         overflow would have occurred.
    static bool TryMultiply(Int32 a, Int32 b, Int32& result)
    {
        int va = static_cast<int>(a);
        int vb = static_cast<int>(b);

        if (va == 0 || vb == 0)
        {
            result = Int32(0);
            return true;
        }

        long long res = static_cast<long long>(va) * static_cast<long long>(vb);
        if (res > Int32::MaxValue || res < Int32::MinValue)
        {
            return false;
        }

        result = Int32(static_cast<int>(res));
        return true;
    }

private:
    /// @brief Private deleted constructor to prevent instantiation.
    ///
    /// The Math class is a static utility class and should not be instantiated.
    Math() = delete;
};

} // namespace System

#endif // SYSTEM_MATH_HPP
