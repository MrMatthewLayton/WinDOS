#ifndef SYSTEM_MATH_HPP
#define SYSTEM_MATH_HPP

#include "Types.hpp"
#include "Exception.hpp"
#include <cstddef>  // for size_t

namespace System
{

/**
 * Provides constants and static methods for common mathematical functions
 * and checked arithmetic operations.
 */
class Math
{
public:
    // ========================================================================
    // Basic Operations
    // ========================================================================

    static Int32 Abs(Int32 value)
    {
        int v = static_cast<int>(value);
        return Int32(v < 0 ? -v : v);
    }

    static Int32 Min(Int32 a, Int32 b)
    {
        int va = static_cast<int>(a);
        int vb = static_cast<int>(b);
        return Int32(va < vb ? va : vb);
    }

    static Int32 Max(Int32 a, Int32 b)
    {
        int va = static_cast<int>(a);
        int vb = static_cast<int>(b);
        return Int32(va > vb ? va : vb);
    }

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

    static void Swap(Int32& a, Int32& b)
    {
        Int32 temp = a;
        a = b;
        b = temp;
    }

    // ========================================================================
    // Checked Arithmetic - throws OverflowException on overflow
    // ========================================================================

    /**
     * Adds two Int32 values with overflow checking.
     * Throws OverflowException if the result would overflow.
     */
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

    /**
     * Subtracts two Int32 values with overflow checking.
     * Throws OverflowException if the result would overflow.
     */
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

    /**
     * Multiplies two Int32 values with overflow checking.
     * Throws OverflowException if the result would overflow.
     */
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

    /**
     * Casts a size_t to Int32 with overflow checking.
     * Throws OverflowException if the value is too large.
     */
    static Int32 CheckedCast(size_t value)
    {
        if (value > static_cast<size_t>(Int32::MaxValue))
        {
            throw OverflowException("Value too large to fit in Int32.");
        }
        return Int32(static_cast<int>(value));
    }

    /**
     * Casts a long long to Int32 with overflow checking.
     * Throws OverflowException if the value is out of range.
     */
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

    /**
     * Tries to add two Int32 values.
     * Returns true if successful, false if overflow would occur.
     */
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

    /**
     * Tries to subtract two Int32 values.
     * Returns true if successful, false if overflow would occur.
     */
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

    /**
     * Tries to multiply two Int32 values.
     * Returns true if successful, false if overflow would occur.
     */
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
    Math() = delete;  // Static class - no instantiation
};

} // namespace System

#endif // SYSTEM_MATH_HPP
