#ifndef SYSTEM_MATH_HPP
#define SYSTEM_MATH_HPP

#include "Types.hpp"
#include "Exception.hpp"

namespace System
{
    /// @brief Provides constants and static methods for common mathematical functions and checked arithmetic operations.
    ///
    /// The Math class is a static utility class that provides basic mathematical operations (Abs, Min, Max, Clamp, Swap)
    /// as well as checked arithmetic operations that detect integer overflow and underflow conditions.
    ///
    /// Checked operations (CheckedAdd, CheckedSubtract, CheckedMultiply) throw OverflowException when overflow would occur,
    /// while Try operations (TryAdd, TrySubtract, TryMultiply) return false instead of throwing.
    ///
    /// This class cannot be instantiated.
    class Math
    {
    public:
        /// @brief Returns the absolute value of a 32-bit signed integer.
        /// @param value The integer value to compute the absolute value of.
        /// @return The absolute value of the input. If value is negative, returns -value;
        ///         otherwise returns value unchanged.
        static Int32 Abs(const Int32 value)
        {
            if (value < Int32::Zero)
                return -value;
            return value;
        }

        /// @brief Returns the smaller of two 32-bit signed integers.
        /// @param left The first value to compare.
        /// @param right The second value to compare.
        /// @return The smaller of the two values. If values are equal, returns that value.
        static Int32 Min(const Int32 left, const Int32 right)
        {
            if (left < right)
                return left;
            return right;
        }

        /// @brief Returns the larger of two 32-bit signed integers.
        /// @param left The first value to compare.
        /// @param right The second value to compare.
        /// @return The larger of the two values. If values are equal, returns that value.
        static Int32 Max(const Int32 left, const Int32 right)
        {
            if (left > right)
                return left;
            return right;
        }

        /// @brief Clamps a value to be within a specified range.
        /// @param value The value to clamp.
        /// @param min The minimum allowed value (inclusive).
        /// @param max The maximum allowed value (inclusive).
        /// @return If value < min, returns min. If value > max, returns max; otherwise returns value unchanged.
        static Int32 Clamp(const Int32 value, const Int32 min, const Int32 max)
        {
            if (value < min)
                return min;

            if (value > max)
                return max;

            return value;
        }

        /// @brief Swaps the values of two 32-bit signed integer references.
        /// @param left Reference to the first value, which will receive b's original value.
        /// @param right Reference to the second value, which will receive a's original value.
        static void Swap(Int32 &left, Int32 &right)
        {
            const Int32 temp = left;

            left = right;
            right = temp;
        }

        /// @brief Adds two Int32 values with overflow checking.
        /// @param left The first addend.
        /// @param right The second addend.
        /// @return The sum of a and b.
        /// @throws OverflowException If the result overflows Int32::MaxValue
        ///         (positive overflow) or underflows Int32::MinValue (negative overflow).
        static Int32 CheckedAdd(const Int32 left, const Int32 right)
        {
            // Check for overflow
            if (right > Int32::Zero && left > Int32::MaxValue - right)
                throw OverflowException("Integer overflow in addition."); // NOLINT(*-exception-baseclass)

            // Check for underflow
            if (right < Int32::Zero && left < Int32::MinValue - right)
                throw OverflowException("Integer underflow in addition."); // NOLINT(*-exception-baseclass)

            return left + right;
        }

        /// @brief Subtracts two Int32 values with overflow checking.
        /// @param left The minuend (value to subtract from).
        /// @param right The subtrahend (value to subtract).
        /// @return The difference (a - b).
        /// @throws OverflowException If the result overflows Int32::MaxValue
        ///         (positive overflow) or underflows Int32::MinValue (negative overflow).
        static Int32 CheckedSubtract(const Int32 left, const Int32 right)
        {
            // Check for overflow
            if (right < Int32::Zero && left > Int32::MaxValue + right)
                throw OverflowException("Integer overflow in subtraction."); // NOLINT(*-exception-baseclass)

            // Check for underflow
            if (right > Int32::Zero && left < Int32::MinValue + right)
                throw OverflowException("Integer underflow in subtraction."); // NOLINT(*-exception-baseclass)

            return left - right;
        }

        /// @brief Multiplies two Int32 values with overflow checking.
        /// @param left The first factor (multiplicand).
        /// @param right The second factor (multiplier).
        /// @return The product of a and b.
        /// @throws OverflowException If the result overflowed Int32::MaxValue or underflow Int32::MinValue.
        static Int32 CheckedMultiply(const Int32 left, const Int32 right)
        {
            if (left == Int32::Zero || right == Int32::Zero)
                return Int32::Zero;

            const Int64 result = left.ToInt64() * right.ToInt64();

            // Check for overflow using 64-bit arithmetic
            if (result > Int32::MaxValue.ToInt64() || result < Int32::MinValue.ToInt64())
                throw OverflowException("Integer overflow in multiplication."); // NOLINT(*-exception-baseclass)

            return result.ToInt32();
        }

        /// @brief Attempts to add two Int32 values without throwing on overflow.
        /// @param left The first addend.
        /// @param right The second addend.
        /// @param result Output parameter that receives the sum if successful. Unchanged if overflow would occur.
        /// @return True if the addition succeeded without overflow; false if overflow had occurred.
        static bool TryAdd(const Int32 left, const Int32 right, Int32 &result)
        {
            if (right > Int32::Zero && left > Int32::MaxValue - right)
                return false;

            if (right < Int32::Zero && left < Int32::MinValue - right)
                return false;

            result = left + right;
            return true;
        }

        /// @brief Attempts to subtract two Int32 values without throwing on overflow.
        /// @param left The minuend (value to subtract from).
        /// @param right The subtrahend (value to subtract).
        /// @param result Output parameter that receives the difference if successful. Unchanged if overflow would occur.
        /// @return True if the subtraction succeeded without overflow; false if overflow had occurred.
        static bool TrySubtract(const Int32 left, const Int32 right, Int32 &result)
        {
            if (right < Int32::Zero && left > Int32::MaxValue + right)
                return false;

            if (right > Int32::Zero && left < Int32::MinValue + right)
                return false;

            result = left - right;
            return true;
        }

        /// @brief Attempts to multiply two Int32 values without throwing on overflow.
        /// @param left The first factor (multiplicand).
        /// @param right The second factor (multiplier).
        /// @param result Output parameter that receives the product if successful.
        ///               Set to 0 if either operand is 0, unchanged otherwise if overflow would occur.
        /// @return True if the multiplication succeeded without overflow; false if overflow had occurred.
        static bool TryMultiply(const Int32 left, const Int32 right, Int32 &result)
        {
            if (left == Int32::Zero || right == Int32::Zero)
            {
                result = Int32(0);
                return true;
            }

            const Int64 res = left.ToInt64() * right.ToInt64();

            if (res > Int32::MaxValue.ToInt64() || res < Int32::MinValue.ToInt64())
                return false;

            result = res.ToInt32();
            return true;
        }

    private:
        /// @brief Private deleted constructor to prevent instantiation.
        ///
        /// The Math class is a static utility class and should not be instantiated.
        Math() = delete; // NOLINT(*-use-equals-delete)
    };
} // namespace System

#endif // SYSTEM_MATH_HPP
