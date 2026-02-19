# Code Warnings Analysis and Fix Plan

Generated: 2026-02-19

## Status: COMPLETE

All ~150 warnings have been fixed. Build now produces **zero warnings**.

---

## Summary

The DJGPP build originally produced **~150+ warnings**, falling into three categories:

| Category | Count | Status |
|----------|-------|--------|
| Ambiguous operator overloads | ~140 | FIXED |
| memcpy with wrapper types | 2 | FIXED |
| Unused variables | 3 | FIXED |

---

## Category 1: Ambiguous Operator Overloads (FIXED)

### Problem

When comparing wrapper types (`Int32`, `UInt32`, etc.) with integer literals (`0`, `1`, `-1`), the compiler saw two valid candidates.

### Solution Applied

Added explicit `int` overloads to `DEFINE_INTEGER_TYPE` macro in `Types.hpp`:

```cpp
// Comparison operators with int
bool operator==(int other) const { return _value == static_cast<UnderlyingType>(other); }
bool operator!=(int other) const { return _value != static_cast<UnderlyingType>(other); }
bool operator<(int other) const { return _value < static_cast<UnderlyingType>(other); }
bool operator>(int other) const { return _value > static_cast<UnderlyingType>(other); }
bool operator<=(int other) const { return _value <= static_cast<UnderlyingType>(other); }
bool operator>=(int other) const { return _value >= static_cast<UnderlyingType>(other); }

// Arithmetic operators with int
ClassName operator+(int other) const { return ClassName(_value + static_cast<UnderlyingType>(other)); }
ClassName operator-(int other) const { return ClassName(_value - static_cast<UnderlyingType>(other)); }
ClassName operator*(int other) const { return ClassName(_value * static_cast<UnderlyingType>(other)); }
ClassName operator/(int other) const { /* with div-by-zero check */ }
ClassName operator%(int other) const { /* with div-by-zero check */ }

// Compound assignment operators with int
ClassName& operator+=(int other) { _value += static_cast<UnderlyingType>(other); return *this; }
ClassName& operator-=(int other) { _value -= static_cast<UnderlyingType>(other); return *this; }
ClassName& operator*=(int other) { _value *= static_cast<UnderlyingType>(other); return *this; }
ClassName& operator/=(int other) { /* with div-by-zero check */ }
ClassName& operator%=(int other) { /* with div-by-zero check */ }
```

**Note:** For unsigned types (UInt32, UInt64, etc.), negative int values wrap to large positive values when cast to the underlying type. This matches standard C++ behavior.

---

## Category 2: memcpy with Non-Trivial Types (FIXED)

### Problem

```
warning: 'void* memcpy(void*, const void*, size_t)' copying an object of
non-trivial type 'class System::UInt32' from an array of 'unsigned int'
```

### Solution Applied

Changed `Display.cpp` FadeIn/FadeOut to use `unsigned int*` instead of `UInt32*` for pixel buffers:

```cpp
// Before:
UInt32* original = static_cast<UInt32*>(std::malloc(...));
std::memcpy(original, pixels, ... * sizeof(UInt32));

// After:
unsigned int* original = static_cast<unsigned int*>(std::malloc(...));
std::memcpy(original, pixels, ... * sizeof(unsigned int));
```

---

## Category 3: Unused Variables (FIXED)

### Problem

Variables set but never used:
- `Forms.cpp:409` - `childScreen` (removed)
- `Forms.cpp:777` - `crossAxisSize` in wrap mode (removed)
- `Drawing.cpp:2213` - `pitch` in FON rendering (removed after column-major fix)

### Solution Applied

Removed unused variables. The `pitch` variable was left over from the row-major FON rendering code; the column-major fix made it unnecessary.

---

## Verification

```bash
$ make clean && make 2>&1 | grep -c "warning:"
0
```

Build now produces zero warnings on DJGPP.

---

## Files Modified

| File | Changes |
|------|---------|
| `src/System/Types.hpp` | Added `int` overloads to DEFINE_INTEGER_TYPE macro |
| `src/System/IO/Devices/Display.cpp` | Changed pixel buffer type from `UInt32*` to `unsigned int*` |
| `src/System/Windows/Forms/Forms.cpp` | Removed unused `childScreen` and `crossAxisSize` variables |
| `src/System/Drawing/Drawing.cpp` | Removed unused `pitch` variable |
