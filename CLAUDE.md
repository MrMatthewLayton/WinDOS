# CLAUDE.md - Project Context for AI Assistants

## Project Overview

**WinDOS** is a .NET-style core library (rtcorlib - Retro Technology Core Library) for DOS, implementing a Windows 95-inspired graphical user interface system that runs in DOS protected mode via DJGPP.

## What We're Building

A complete GUI framework for DOS that mimics both:
1. **.NET Framework APIs** - Type wrappers, String, Array<T>, Exception hierarchy, Console
2. **Windows 95 UI** - Desktop, TaskBar, Windows, Buttons, and a WIMP (Windows, Icons, Menus, Pointer) environment

The goal is to create a nostalgic yet functional DOS GUI system using modern C++17, with APIs familiar to .NET developers.

## Build System

```bash
# Compiler: DJGPP cross-compiler for DOS
CXX = ~/djgpp/bin/i586-pc-msdosdjgpp-g++

# Build commands
make              # Build rtcorlib library (librtcorlib.a)
make test         # Build comprehensive test suite (test.exe)
make tests        # Build all test executables
make demo         # Build graphics demo
make forms_demo   # Build Windows 95-style forms demo
make clean        # Clean build artifacts
```

**Output:**
- Library: `build/lib/librtcorlib.a`
- Tests: `build/bin/test.exe` (comprehensive), `build/bin/test_*.exe` (individual)
- Demos: `build/bin/gfxdemo.exe`, `build/bin/forms.exe`

## Project Structure

```
windos/
├── src/                        # Source code (organized by namespace)
│   ├── rtcorlib.hpp            # Master include file
│   ├── Platform/               # Low-level DOS abstractions
│   │   ├── Platform.hpp
│   │   └── DOS/
│   │       ├── System.hpp/.cpp
│   │       ├── Video.hpp/.cpp
│   │       ├── Graphics.hpp/.cpp
│   │       ├── Keyboard.hpp/.cpp
│   │       └── Mouse.hpp/.cpp
│   └── System/                 # System namespace
│       ├── Types.hpp/.cpp      # Boolean, Char, Int*, Float*
│       ├── Exception.hpp/.cpp  # Exception hierarchy
│       ├── String.hpp/.cpp     # Immutable String class
│       ├── Array.hpp           # Generic Array<T> template
│       ├── Console.hpp/.cpp    # Console I/O with colors
│       ├── Math.hpp            # Math utilities
│       ├── Memory.hpp/.cpp     # MemoryPool, StringIntern
│       ├── IO/                 # System.IO namespace
│       │   └── IO.hpp/.cpp     # File class
│       ├── Drawing/            # System.Drawing namespace
│       │   └── Drawing.hpp/.cpp
│       ├── Devices/            # System.Devices namespace
│       │   └── Devices.hpp/.cpp
│       └── Windows/
│           └── Forms/          # System.Windows.Forms namespace
│               └── Forms.hpp/.cpp
├── tests/                      # Tests and demos
│   ├── test_framework.hpp
│   ├── test.cpp                # Comprehensive test suite
│   ├── test_*.cpp              # Individual unit tests
│   ├── gfx_demo.cpp
│   └── forms_demo.cpp
├── assets/                     # Asset files (bitmaps, icons)
├── build/                      # Build output directory
├── Makefile
├── CLAUDE.md                   # Project context
└── WinDOS.md                   # API documentation
```

---

# CODE REVIEW & SCORING

## Overall Scores

| Category | Score | Notes |
|----------|-------|-------|
| **Quality** | 90/100 | Clean structure, consistent APIs, well-documented |
| **Performance** | 88/100 | StringBuilder, MemoryPool, StringIntern, transparency |
| **Security** | 90/100 | Bounds checking, checked arithmetic, proper exceptions |
| **Pitfalls** | 94/100 | All critical and medium issues resolved |

---

## Quality Assessment (85/100)

### Strengths
- Clean namespace organization mirroring .NET
- Consistent coding style throughout
- Good use of RAII for resource management
- Move semantics implemented correctly
- Well-structured exception hierarchy
- Comprehensive test coverage
- **[FIXED] Wrapper types (Int32, Boolean, Char) used consistently EVERYWHERE**
- **[FIXED] Mixed-type operators for arithmetic and comparison (Int32 + int, etc.)**

### Issues

#### Resolved: Type Consistency
~~The codebase inconsistently uses primitives instead of wrapper types.~~

**Status:** FIXED - Wrapper types are now used throughout the entire codebase, not just public APIs. Added mixed-type arithmetic and comparison operators to Types.hpp (e.g., `Int32 + int`, `Int32 == 0` work without explicit casts). See "Wrapper Type System" section for details.

---

## Performance Assessment (88/100)

### Strengths
- Lookup table for chunky-to-planar conversion (Drawing.cpp:109-126)
- Dirty rectangle optimization for VGA updates
- Fast memset-based rectangle filling
- Efficient Bresenham line algorithm
- Move semantics prevent unnecessary copies
- **[NEW] Image::CopyFrom with transparency support for sprite rendering**
- **[NEW] StringBuilder class for efficient string building**
- **[NEW] MemoryPool class for fast fixed-size allocations**
- **[NEW] StringIntern class for string deduplication and O(1) comparison**

### Issues

#### P1/P2: String Operations - MITIGATED
~~String operations create excessive allocations; += is O(n) per append.~~

**Status:** MITIGATED - Added `StringBuilder` class with:
- Pre-allocated buffer with capacity-based growth
- `Append()` methods with O(1) amortized complexity
- Method chaining support: `sb.Append("Hello").Append(' ').Append("World")`
- `AppendLine()` for line-based building
- `Insert()`, `Remove()`, `Clear()` for buffer manipulation

String class remains immutable by design (like .NET), use StringBuilder for loops.

#### P3: Control Hierarchy Traversal - FIXED
~~Linear search through children for hit testing.~~

**Status:** FIXED - Added `SpatialGrid` class with:
- 64x64 pixel cells for O(1) average hit testing
- `Insert()`, `Remove()`, `HitTest()` methods
- Desktop now uses spatial grid for fast child lookups
- Falls back to linear search for edge cases

#### P4: Image::CopyFrom Without Transparency - FIXED
~~Currently copies all pixels. Needs transparent pixel skipping for sprites.~~

**Status:** FIXED - Added `CopyFrom(src, x, y, transparentColor)` overload that skips pixels matching the transparent color.

---

## Security Assessment (87/100)

### Strengths
- Array bounds checking with exceptions
- String index validation
- Null pointer checks in most places
- Division by zero protection in wrapper types
- **[NEW] Image::FromBitmap throws descriptive exceptions (FileNotFoundException, InvalidDataException)**
- **[NEW] Added IOException, FileNotFoundException, InvalidDataException to exception hierarchy**

### Issues

#### S1: Integer Overflow Protection - FIXED
~~No overflow protection in String operations.~~

**Status:** FIXED - Added `System::Math` class with:
- `CheckedAdd()`, `CheckedSubtract()`, `CheckedMultiply()` - throw `OverflowException`
- `TryAdd()`, `TrySubtract()`, `TryMultiply()` - return success/failure
- String operations (Replace, operator+) now use checked arithmetic
- Type conversions use `To*()` methods on integer types (e.g., `x.ToInt32()`)

#### S2: File Path Injection (Low Risk in DOS)
```cpp
// Drawing.cpp:311
FILE* file = std::fopen(path, "rb");
```
No path validation. Low risk in DOS environment but should validate.

#### S3: Unchecked malloc Returns
```cpp
// Drawing.cpp:137
_data = static_cast<unsigned char*>(std::malloc(size));
if (_data) { ... }  // Good - checked
```
Most malloc calls are checked, but verify all are.

---

## Pitfalls Assessment (94/100)

### Resolved Issues

#### PIT-1: Hardcoded Screen Dimensions - FIXED
~~Screen size was hardcoded throughout (640x480).~~

**Status:** FIXED - Desktop now queries `Display::GetCurrent()` for screen dimensions and caches them. All Forms components (Control, Desktop, TaskBar) use dynamic screen dimensions. Added `Desktop::ScreenWidth()` and `Desktop::ScreenHeight()` accessors.

#### PIT-3: Window Type Check by Position - FIXED
~~Dangerous static_cast based on Y position to determine if a control was a Window.~~

**Status:** FIXED - Added comprehensive type identification system:
- `ControlType` enum for runtime type identification
- `GetControlType()` virtual method (similar to .NET `GetType()`)
- `AsWindow()`, `AsButton()`, etc. safe cast methods (similar to .NET `as` operator)
- `IsWindow()`, `IsButton()`, etc. type check methods (similar to .NET `is` operator)
- HandleMouse now uses `child->AsWindow()` instead of position-based casting

#### PIT-2: Control Destructor Deletes Children - DOCUMENTED
~~Undocumented ownership behavior could lead to crashes.~~

**Status:** DOCUMENTED - Added comprehensive ownership documentation:
- Block comment in Forms.hpp Control class explaining ownership rules
- Comments on AddChild/RemoveChild methods
- Full "Ownership Semantics" section in WinDOS.md with correct/incorrect examples
- Expanded notes in CLAUDE.md Important Notes section

This is intentional behavior (composite pattern), now properly documented.

### Remaining Issues

None critical. Minor items remain in MEDIUM/LOW priority.

#### PIT-4: Static Frame Buffer
```cpp
// Drawing.cpp:564
GraphicsBuffer* GraphicsBuffer::_frameBuffer = nullptr;
```
Global state makes testing difficult and prevents multiple displays.

#### PIT-5: Missing Virtual Destructor Warning
All control classes have virtual destructors (good), but ensure all base classes do.

---

## Outstanding TODO Items

### HIGH PRIORITY - Must Fix Before Production

1. **[x] Type Consistency** - Replace primitive types with wrapper types in all public APIs ✓ DONE
2. **[x] Screen Size Constants** - Remove hardcoded 640x480; use Display properties ✓ DONE
3. **[x] Safe Window Casting** - Add type identification to Control hierarchy ✓ DONE
4. **[x] Document Ownership** - Clear documentation on child control ownership ✓ DONE

### MEDIUM PRIORITY - Should Fix

5. **[x] String Performance** - StringBuilder class implemented ✓ DONE
6. **[x] Transparency Support** - Image::CopyFrom with transparent color ✓ DONE
7. **[x] Integer Overflow** - Math class with checked arithmetic ✓ DONE
8. **[x] Error Handling** - Image::FromBitmap throws exceptions ✓ DONE

### LOW PRIORITY - Nice to Have

9. **[x] Spatial Partitioning** - SpatialGrid class with 64px cells for O(1) hit testing ✓ DONE
10. **[x] String Interning** - StringIntern class with hash table pool ✓ DONE
11. **[x] Memory Pool** - MemoryPool class for fixed-size allocations ✓ DONE

---

## Features Ported from Legacy Code

All HIGH and MEDIUM priority features have been ported from legacy code.

### COMPLETED
| Feature | Location | Description |
|---------|----------|-------------|
| ~~File I/O System~~ | System/IO/IO.hpp | `File::ReadAllBytes()`, `File::Exists()`, `File::GetSize()` ✓ DONE |
| ~~Icon Loading~~ | System/Drawing/Drawing.hpp | `Image::FromIcon()`, `Image::FromIconLibrary()` with scaling and named icon support ✓ DONE |
| ~~Desktop Icons~~ | System/Windows/Forms/Forms.hpp | `Desktop::AddIcon()`, `DesktopIcon` struct, grid layout ✓ DONE |
| ~~Palette Fade~~ | System/Devices/Devices.hpp | `Display::FadeIn(ms)`, `FadeOut(ms)` - VGA and VBE modes ✓ DONE |
| ~~Custom Cursors~~ | System/Windows/Forms/Forms.hpp | `Desktop::SetCursor()`, `LoadCursorFromLibrary()` ✓ DONE |
| ~~Hatch Patterns~~ | System/Drawing/Drawing.hpp | 38 fill patterns in `HatchStyle` class ✓ DONE |
| ~~Pattern Fills~~ | System/Drawing/Drawing.hpp | `Graphics::FillRectangle(rect, HatchStyle, fg, bg)` ✓ DONE |
| ~~Math Utilities~~ | System/Math.hpp | `Abs()`, `Min()`, `Max()`, `Clamp()`, `Swap()` ✓ DONE |

### LOW PRIORITY - Remaining
| Feature | Source | Description |
|---------|--------|-------------|
| String Formatting | text.h | `StringFormatter::Format()` with varargs |
| MutableArray | collect.h | `InsertAt()`, `RemoveAt()` |
| ASCII Constants | text.h | `ASCII_ESCAPE`, `ASCII_TAB`, etc. |

---

## Code Conventions

- C++17 features (if constexpr, structured bindings, nested namespaces)
- Allman brace style (opening brace on its own line)
- RAII for resource management
- Copy/move semantics throughout
- Bounds checking with exceptions
- Static methods for hardware access facades
- Virtual methods for control customization
- **USE WRAPPER TYPES EVERYWHERE** - See "Wrapper Type System" section below
- **DOXYGEN DOCUMENTATION** - All public APIs must have Doxygen comments
- **NO REDUNDANT ACCESS SPECIFIERS** - Classes default to `private`, so omit initial `private:`
  ```cpp
  // Correct - no redundant private:
  class Foo
  {
      int _value;  // private by default

  public:
      int GetValue() const;
  };

  // Incorrect - redundant private:
  class Foo
  {
  private:        // <-- unnecessary, class defaults to private
      int _value;

  public:
      int GetValue() const;
  };
  ```

## Documentation Standards

All public classes, methods, and functions **MUST** have Doxygen documentation using the `///` format:

```cpp
/// @brief Computes the sum of two integers.
/// @param a The first operand.
/// @param b The second operand.
/// @return The sum of a and b.
/// @throws OverflowException If the result overflows Int32.
Int32 Add(Int32 a, Int32 b);
```

### Required Tags

| Tag | Usage |
|-----|-------|
| `@brief` | One-line description (required for all) |
| `@param` | Document each parameter |
| `@return` | Document return value (if non-void) |
| `@throws` | Document exceptions that may be thrown |
| `@note` | Important usage notes |
| `@see` | Cross-references to related APIs |
| `@example` | Usage examples for complex APIs |

### Documentation Scope

- **Classes**: Document purpose, ownership semantics, thread safety
- **Constructors**: Document initialization behavior and parameters
- **Methods**: Document behavior, parameters, return values, exceptions
- **Properties/Getters**: Brief description of what is returned
- **Constants**: Document meaning and valid usage

### Style Guidelines

- Use `[[nodiscard]]` for getters and methods whose return value should not be ignored
- Keep `@brief` to one line; use additional paragraphs for details
- Document preconditions and postconditions where relevant
- Use `@code` / `@endcode` for inline code examples

## Testing

Run comprehensive tests:
```bash
make test
# Copy test.exe and CWSDPMI.EXE to DOS environment
# Run: test.exe
```

Tests use a simple framework in `test_framework.hpp`:
```cpp
ASSERT(condition, "message");
ASSERT_EQ(expected, actual, "message");
ASSERT_THROWS(expression, ExceptionType, "message");
```

## Important Notes for Development

1. **VGA Planar Mode**: Mode 0x12 uses 4 bit planes. Each pixel requires writing to multiple planes via port I/O.

2. **Coordinate Systems**:
   - Control bounds are relative to parent
   - Use `ScreenBounds()` for absolute coordinates
   - Client area excludes borders/title bars

3. **Memory**: DOS protected mode via CWSDPMI.EXE provides 32-bit flat memory model.

4. **File Paths**: Use backslashes in DOS paths (`assets\\test.bmp`).

5. **No Exceptions in Destructors**: Destructors don't throw; cleanup is silent.

6. **Child Control Ownership**: Parent controls OWN and DELETE their children in destructor.
   - Always use `new` to allocate children: `new Button(parent, bounds)`
   - Never allocate children on the stack - causes double-free crash
   - Never manually delete children that have a parent
   - `RemoveChild()` releases ownership - caller must then delete or re-parent
   - See `WinDOS.md` Control class documentation for full details

---

## Wrapper Type System (Complete)

### Overview

The codebase uses .NET-style wrapper types **everywhere**, not just in public APIs. This provides type safety, zero-cost abstractions (with optimization), and consistency with .NET conventions.

### Wrapper Types Available

| Wrapper | Underlying Type | Usage |
|---------|-----------------|-------|
| `Boolean` | `bool` | All boolean values |
| `Char` | `char` | Single characters |
| `Int8` / `SByte` | `std::int8_t` | 8-bit signed integers |
| `UInt8` / `Byte` | `std::uint8_t` | 8-bit unsigned, pixel data |
| `Int16` / `Short` | `std::int16_t` | 16-bit signed integers |
| `UInt16` / `UShort` | `std::uint16_t` | 16-bit unsigned integers |
| `Int32` / `Int` | `std::int32_t` | **Primary integer type** |
| `UInt32` / `UInt` | `std::uint32_t` | Unsigned 32-bit, colors |
| `Int64` / `Long` | `std::int64_t` | 64-bit signed integers |
| `UInt64` / `ULong` | `std::uint64_t` | 64-bit unsigned integers |
| `UIntPtr` | `std::uintptr_t` | Pointer-sized unsigned (32-bit on DOS) |
| `Float32` / `Single` | `float` | 32-bit floating point |
| `Float64` / `Double` | `double` | 64-bit floating point |

All integer types use fixed-width types from `<cstdint>` for guaranteed sizes.

### Integer Type Constants

All integer types have these static constants (as wrapper types, enabling method chaining):
```cpp
Int32::MinValue    // -2147483648 as Int32
Int32::MaxValue    // 2147483647 as Int32
Int32::Zero        // 0 as Int32

// Method chaining works:
Int32::MaxValue.ToInt64()  // Convert max value to Int64
```

### Type Conversion Methods

All integer types have overflow-checked conversion methods:
```cpp
Int32 x = Int32(1000);
Int64 big = x.ToInt64();      // Always safe (widening)
Int8 small = x.ToInt8();      // Throws OverflowException (1000 > 127)
UInt32 u = x.ToUInt32();      // Works (positive value)

Int32 neg = Int32(-5);
UInt32 fail = neg.ToUInt32(); // Throws OverflowException (negative)

// Available methods on all integer types:
// ToInt8(), ToUInt8(), ToInt16(), ToUInt16()
// ToInt32(), ToUInt32(), ToInt64(), ToUInt64(), ToUIntPtr()

// Generic template also available:
auto result = x.To<Int64>();
```

### Usage Rules

1. **Use wrapper types for all variables**:
   ```cpp
   // Correct
   Int32 count = Int32(0);
   for (Int32 i = Int32(0); i < limit; i += 1) { ... }

   // Incorrect
   int count = 0;
   for (int i = 0; i < limit; i++) { ... }
   ```

2. **Use `static_cast<int>()` for array indexing**:
   ```cpp
   Int32 index = Int32(5);
   array[static_cast<int>(index)] = value;  // Array indexing needs int
   ```

3. **Use `static_cast` at platform boundaries**:
   ```cpp
   // Platform functions take int& references
   Int32 row, col;
   { int r, c; Platform::DOS::Video::GetCursorPosition(r, c); row = Int32(r); col = Int32(c); }
   ```

4. **Loop increment pattern**:
   ```cpp
   // Use += 1 instead of ++ for clarity with wrapper types
   for (Int32 i = Int32(0); i < limit; i += 1) { ... }
   ```

### Types That REMAIN Primitives

Some types must stay as primitives for compatibility:

| Primitive | Reason |
|-----------|--------|
| `size_t` | Required by C stdlib (malloc, strlen, fread) - use `UIntPtr` wrapper when possible |
| `long` | Required by ftell() return type |
| `unsigned short` | VBE mode numbers (hardware interface) |
| `const char*` | C string interop |
| Pointers | Low-level memory management |
| `int` for array indexing | C++ array subscript requirement |
| `unsigned char` in static data | Palette/pattern storage |

### Mixed-Type Operators

Types.hpp includes operators for mixed-type arithmetic to avoid ambiguity:
```cpp
// These all work without explicit casts:
Int32 x = Int32(5);
Int32 y = x + 1;      // Int32 + int → Int32
Int32 z = x * 2;      // Int32 * int → Int32
Boolean b = x > 0;    // Int32 > int → Boolean
```

### Zero Overhead

With compiler optimizations enabled (`-O2`), wrapper types compile to identical machine code as primitives. The wrapper disappears entirely - `Int32 a + Int32 b` generates the same assembly as `int a + int b`.

### Files Updated

All source files use wrapper types internally:
- `String.hpp/.cpp` - `Int32 _length`, all methods
- `StringBuilder` - `Int32 _length`, `_capacity`
- `Console.hpp/.cpp` - All internals
- `Memory.hpp/.cpp` - Pool sizes, hash values
- `IO.cpp` - Loop counters, lengths
- `Drawing.cpp` - Coordinates, pixel operations
- `Devices.cpp` - Display operations
- `Forms.cpp` - All control classes

---

## VBE 2.0+ High-Resolution Support (Complete)

### Overview

VESA BIOS Extensions (VBE) 2.0+ support for high-resolution modes (800x600+) with linear framebuffer access. Fully integrated with the unified 32bpp graphics pipeline.

### What Was Implemented

#### Platform Layer (`src/Platform/DOS/Graphics.hpp/.cpp`)
- `VbeInfoBlock` and `VbeModeInfoBlock` structs for VBE BIOS data
- `VbeSurface` struct to track LFB selector, address, pitch, dimensions
- `DetectVBE()` - INT 10h AX=4F00h to detect VBE presence
- `GetVBEModeInfo()` - INT 10h AX=4F01h to query mode capabilities
- `SetVBEMode()` - INT 10h AX=4F02h with LFB mapping via DPMI
- `CleanupVBE()` - Proper resource cleanup (LDT descriptor, physical mapping)
- `GetLfbSelector()` - Returns LDT selector for LFB access

#### Drawing Layer (`src/System/Drawing/Drawing.hpp/.cpp`)
- Unified `Color` class (32-bit ARGB) with `Lerp()` for gradients
- Unified `Image` class (32-bit ARGB pixels)
- `GraphicsBuffer::CreateFrameBuffer32()` with bpp parameter
- `Linear32BufferWriter()` - Copies framebuffer to LFB via `movedata()`
- Supports both 24bpp and 32bpp VBE modes
- Bayer dithering for 4bpp/8bpp display modes

#### Device Layer (`src/System/Devices/Devices.hpp/.cpp`)
- `Display::IsVbeAvailable()` - Checks for VBE 2.0+
- `Display::DetectVbeMode()` - Finds suitable VBE mode
- `Display::SetMode()` updated to handle VBE modes with LFB setup

#### Forms Layer (`src/System/Windows/Forms/Forms.hpp/.cpp`)
- `SpectrumControl` - Vertical gradient widget (white→color→black)
- Uses unified `Color` and `Image` classes

### Key Technical Findings

#### 1. Transfer Buffer Corruption
**Problem:** DJGPP's `__tb` transfer buffer is shared with DOS I/O (printf, etc.).
**Solution:** Copy data from transfer buffer IMMEDIATELY after BIOS calls, before any printf.
```cpp
__dpmi_int(0x10, &regs);
dosmemget(tbAddr, sizeof(VbeInfoBlock), &vbeInfo);  // Copy FIRST
printf("Result: ...");  // printf can now safely use __tb
```

#### 2. LDT Selector Approach (Not Near Pointers)
**Problem:** `__djgpp_nearptr_enable()` causes page faults with some DPMI hosts.
**Solution:** Use LDT descriptor with `movedata()`:
```cpp
int selector = __dpmi_allocate_ldt_descriptors(1);
__dpmi_set_segment_base_address(selector, memInfo.address);
__dpmi_set_segment_limit(selector, lfbSize - 1);
movedata(_my_ds(), srcOffset, selector, dstOffset, rowBytes);
```

#### 3. 24bpp vs 32bpp Mode Detection
**Problem:** VBE mode 0x115 is often 24bpp (3 bytes/pixel), not 32bpp.
**Solution:** Track actual bpp and convert pixel data accordingly:
- 32bpp: Direct copy (4 bytes/pixel)
- 24bpp: Convert ARGB to BGR (3 bytes/pixel)

#### 4. Unified 32bpp Pipeline (Implemented)
**Problem:** Dual 8-bit/32-bit buffer approach caused compositing complexity and artifacts.
**Solution:** Unified 32bpp pipeline - all drawing uses single 32-bit `Image`. Bayer dithering converts to VGA palette indices only at display time.

### Current Status: COMPLETE

**What Works:**
- VBE mode detection and initialization (800x600 and higher)
- Unified 32bpp graphics pipeline throughout
- Single `Color` class (32-bit ARGB) - matches .NET `System.Drawing.Color`
- Single `Image` class (32-bit pixels)
- Bayer dithering for 4bpp/8bpp VGA modes
- Direct output to 24bpp/32bpp VBE linear framebuffer
- SpectrumControl with smooth gradients
- All Forms controls work correctly in all display modes

### Files Modified for VBE Support

| File | Changes |
|------|---------|
| `src/Platform/DOS/Graphics.hpp` | VBE structs, VbeSurface, function declarations |
| `src/Platform/DOS/Graphics.cpp` | VBE detection, mode setting, LFB mapping |
| `src/System/Drawing/Drawing.hpp` | Unified Color/Image (32-bit), CreateFrameBuffer32 |
| `src/System/Drawing/Drawing.cpp` | Bayer dithering, Linear32BufferWriter, 24/32bpp conversion |
| `src/System/Devices/Devices.hpp` | Display VBE members and methods |
| `src/System/Devices/Devices.cpp` | VBE mode detection and SetMode |
| `src/System/Windows/Forms/Forms.hpp` | SpectrumControl class |
| `src/System/Windows/Forms/Forms.cpp` | SpectrumControl implementation |
| `tests/forms_demo.cpp` | VBE mode usage |
| `tests/vbetest.cpp` | Standalone VBE diagnostic tool |
| `tests/vbebcl.cpp` | VBE test using rtcorlib library |
| `tests/vbeform.cpp` | Minimal VBE forms test |

### Test Programs

```bash
make vbe_debug   # Standalone VBE step-by-step test (vbetest.exe)
make vbe_rtcorlib # VBE test with rtcorlib graphics (vbebcl.exe)
make vbe_form    # Minimal VBE forms test (vbeform.exe)
make forms_demo  # Full forms demo with VBE (forms.exe)
```

### QEMU Configuration for VBE

Use `-vga std` for proper VBE 2.0+ emulation:
```bash
qemu-system-i386 -vga std -m 64 ...
```

Note: `-device VGA` doesn't properly emulate VBE info block responses.

---

## Flexbox-like Layout System (Complete)

### Overview

A CSS Flexbox-inspired layout system that automatically calculates and assigns control positions using a two-pass algorithm (Measure + Arrange). This solves the problem of hardcoded screen coordinates and enables responsive UI design.

### What Was Implemented

#### Layout Enums (`src/System/Windows/Forms/Forms.hpp`)
- `FlexDirection` - Row or Column layout direction
- `JustifyContent` - Main axis alignment (Start, Center, End, SpaceBetween, SpaceAround)
- `AlignItems` - Cross axis alignment (Start, Center, End, Stretch)
- `SizeMode` - How size is determined (Auto, Fixed, Fill)

#### LayoutProperties Struct
Embedded in each Control to configure layout behavior:
```cpp
struct LayoutProperties {
    FlexDirection direction;      // Row or Column
    JustifyContent justifyContent;// Main axis positioning
    AlignItems alignItems;        // Cross axis alignment
    Int32 gap;                    // Space between children
    Int32 flexGrow;               // Proportional growth factor
    Int32 flexShrink;             // Proportional shrink factor
    SizeMode widthMode, heightMode;
    Int32 minWidth, minHeight, maxWidth, maxHeight;
    Int32 marginTop, marginRight, marginBottom, marginLeft;
    Int32 paddingTop, paddingRight, paddingBottom, paddingLeft;
    bool participatesInLayout;    // false = floating (Windows)
    bool needsLayout;             // Dirty flag
};
```

#### Control Layout Methods
- `Layout()` - Access layout properties
- `Measure(availableWidth, availableHeight)` - Bottom-up size calculation
- `Arrange(finalBounds)` - Top-down positioning
- `PerformLayout()` - Trigger full layout pass
- `InvalidateLayout()` - Mark as needing layout
- `GetPreferredSize()` - Override for content-based sizing

### Layout Algorithm

**Phase 1: Measure (bottom-up)**
- Each control reports preferred size based on content/children
- Children measured first, then parent accumulates
- Respects min/max constraints

**Phase 2: Arrange (top-down)**
- Parent assigns final bounds to children based on:
  - Available space
  - FlexGrow distribution
  - JustifyContent positioning
  - AlignItems alignment

### Key Design Decisions

1. **No RTTI**: Uses existing ControlType enum for type identification
2. **No STL**: Uses existing Array<T> template
3. **Dirty flag optimization**: Only recalculate when `needsLayout = true`
4. **Backward compatible**: Default settings preserve existing behavior
5. **Fluent setters**: `control.Layout().SetFlexGrow(1).SetMargin(4)`
6. **Windows are floating**: `participatesInLayout = false` by default

### Files Modified

| File | Changes |
|------|---------|
| `src/System/Windows/Forms/Forms.hpp` | Layout enums, LayoutProperties, MeasureResult, Control layout methods |
| `src/System/Windows/Forms/Forms.cpp` | Measure(), Arrange(), ArrangeFlexChildren(), PerformLayout(), control updates |
| `tests/test_layout.cpp` | Comprehensive test suite (59 tests) |
| `tests/forms_demo.cpp` | Updated to demonstrate layout system |

### Test Results

All 59 layout tests pass:
- Layout properties defaults and fluent API
- Measure pass (single control, fixed size, min/max, row/column)
- Arrange pass (positioning, FlexGrow, JustifyContent, AlignItems)
- Floating controls, nested layout, padding, margins

### Usage Example

```cpp
// Window with row layout for spectrum controls
window->Layout().SetDirection(FlexDirection::Row)
                .SetJustifyContent(JustifyContent::SpaceAround)
                .SetAlignItems(AlignItems::Stretch)
                .SetPadding(10);

// Child controls with flex grow
SpectrumControl* red = new SpectrumControl(window, bounds, Color::Red);
red->Layout().SetFlexGrow(1).SetMargin(5);

SpectrumControl* green = new SpectrumControl(window, bounds, Color::Green);
green->Layout().SetFlexGrow(1).SetMargin(5);

// Trigger layout calculation
window->PerformLayout();
```

---

## Outstanding TODO Items (Updated)

### HIGH PRIORITY

12. **[x] Unified 32bpp Graphics Pipeline** - ✓ DONE
    - Unified `Color` class (32-bit ARGB), old 8-bit removed
    - Unified `Image` class (32-bit pixels), old 8-bit removed
    - All Graphics methods use 32-bit Color/Image
    - Bayer dithering for 4bpp/8bpp display modes
    - Single framebuffer, mode-specific flush

13. **[ ] VGA Palette Control** - Allow setting custom palette colors
    - `Display::SetPaletteColor(index, r, g, b)`
    - `Display::SetPalette(colors[256])`
    - Support for 4bpp (16 colors) and 8bpp (256 colors) modes
    - Useful for optimizing dithering with application-specific palettes

14. **[x] Flexbox-like Layout System** - ✓ DONE
    - Two-pass layout algorithm (Measure + Arrange)
    - FlexDirection, JustifyContent, AlignItems, FlexGrow
    - Padding, margins, gaps, min/max constraints
    - Floating controls (Windows don't participate in layout)
    - 59 unit tests passing

### MEDIUM PRIORITY

15. **[x] Fix SpectrumControl in VBE mode** - ✓ DONE (resolved by unified 32bpp pipeline)

16. **[x] Fix StartMenu hover highlighting** - ✓ DONE
    - Menu items now properly highlight blue on hover
    - Items revert to grey when mouse leaves
    - Fixed by propagating mouse events to all menu items

### COMPLETED - TaskBar Window List

17. **[x] TaskBar window buttons** - ✓ DONE
    - Window buttons automatically appear in taskbar when windows are created
    - Clicking a taskbar button focuses the corresponding window
    - Active window's button shows pressed/sunken state
    - Button states properly separated: toggle state (persistent) vs mouse state (temporary)
    - Fixed virtual dispatch issue: Window constructor registers with taskbar after base class construction

18. **[x] Button toggle state** - ✓ DONE
    - Start button stays pressed while start menu is open
    - Window buttons stay pressed while their window is focused
    - Added `_isToggled` and `_isMouseDown` separation in Button class
    - Visual state = toggled OR mouse down

### COMPLETED - Recent Features

24. **[x] Named Icon Support** - ✓ DONE
    - `Image::FromIconLibrary(path, iconName, size)` - Load icons by name from PE resources
    - `Image::GetIconLibraryNames(path)` - Get all icon names in a library
    - `Image::GetIconLibraryIndex(path, iconName)` - Get index for a named icon
    - `SystemIcons` class with 98 named constants (Computer, FolderOpen, etc.)
    - Parses PE resource directory for RT_GROUP_ICON named entries
    - UTF-16LE name strings converted to ASCII

25. **[x] VBE 3.0 Gamma Ramp Support** - ✓ DONE (future compatibility)
    - `Display::IsGammaSupported()` - Check if VBE 3.0 gamma is available
    - `Display::SetGammaScale(scale)` - Set brightness via gamma table
    - Platform layer: `IsGammaSupported()`, `SetGammaTable()`, `GetGammaTable()`
    - Uses INT 10h AX=4F15h (VBE 3.0 gamma/DAC functions)
    - Note: QEMU's vgabios only implements VBE 2.0; gamma functions return unsupported
    - FadeIn/FadeOut attempts gamma ramp first, falls back to pixel processing

### LOW PRIORITY - Layout System Future Work

26. **[ ] Wrap support** - Add `wrap` property for multi-line layouts
    - When children exceed container, wrap to next line
    - Similar to CSS `flex-wrap`

27. **[ ] Layout animations** - Smooth transitions when layout changes
    - Interpolate between old and new positions
    - Useful for window minimize/maximize effects

28. **[ ] Auto-layout on resize** - Trigger layout when control size changes
    - Currently requires manual PerformLayout() call
    - Could integrate with SetBounds() automatically

29. **[ ] Layout for TaskBar buttons** - Use flexbox for auto-arrangement
    - Currently uses manual positioning in AddWindowButton/RemoveWindowButton
    - Could use Row layout with FlexGrow for automatic distribution

30. **[ ] StartMenu/MenuItem relative positioning** - Refactor to use layout system
    - Currently uses some hardcoded coordinates
    - Could benefit from Column layout for menu items

---

## Named Icon Support (Complete)

### Overview

PE-format icon libraries (.icl, .dll, .exe) store icons with both numeric IDs and string names. The named icon support allows loading icons by their human-readable names instead of opaque indices.

### What Was Implemented

#### Image Class Extensions (`src/System/Drawing/Drawing.hpp/.cpp`)
- `FromIconLibrary(path, iconName, size)` - Load icon by name with automatic scaling
- `GetIconLibraryNames(path)` - Returns Array<String> of all named icons
- `GetIconLibraryIndex(path, iconName)` - Get numeric index for a named icon
- PE resource directory parsing for RT_GROUP_ICON entries
- UTF-16LE name string decoding

#### SystemIcons Class (`src/System/Drawing/Drawing.hpp`)
Provides 98 named constants for the standard icon library:
```cpp
class SystemIcons {
public:
    static constexpr const char* LibraryPath = "sysicons.icl";

    // Hardware icons
    static constexpr const char* Computer = "computer";
    static constexpr const char* ComputerNet = "computer-net";
    static constexpr const char* Disk35 = "disk-35";
    static constexpr const char* DiskHard = "disk-hard";
    static constexpr const char* DiskCD = "disk-cd";

    // Folder icons
    static constexpr const char* Folder = "folder";
    static constexpr const char* FolderOpen = "folder-open";
    static constexpr const char* FolderApps = "folder-apps";
    static constexpr const char* FolderDocs = "folder-docs";

    // File icons
    static constexpr const char* FileBlank = "file-blank";
    static constexpr const char* FileTxt = "file-txt";
    static constexpr const char* FileExe = "file-exe";

    // UI icons
    static constexpr const char* CursorPointer = "cursor-pointer";
    static constexpr const char* StartFlag = "start-flag";
    // ... 83 more icons

    static Image Load(const char* iconName, const Size& size);
};
```

#### Desktop Integration (`src/System/Windows/Forms/Forms.hpp/.cpp`)
- `Desktop::LoadCursorFromLibrary(path, iconName)` - Load cursor by name
- `Desktop::AddIconFromLibrary(path, iconName)` - Add desktop icon by name
- `StartMenu::LoadIcons()` - Uses SystemIcons for menu item icons

### PE Resource Directory Structure

Icon libraries use the PE (Portable Executable) format with a resource section:
```
.rsrc section
└── Root Directory
    └── Type Directory (RT_GROUP_ICON = 14)
        ├── Named Entry: bit 31 set in nameOrId → offset to UTF-16LE string
        │   └── Language Directory → Data Entry → GRPICONDIR
        └── ID Entry: bit 31 clear → numeric ID
            └── Language Directory → Data Entry → GRPICONDIR
```

Name strings are length-prefixed UTF-16LE (2-byte little-endian characters).

### Usage Example

```cpp
using namespace System::Drawing;

// Load icon by name with specific size
Image computerIcon = SystemIcons::Load(SystemIcons::Computer, Size::Icon32);

// Or load directly from any icon library
Image customIcon = Image::FromIconLibrary("myicons.icl", "my-icon-name", Size(48, 48));

// Get all available icon names
Array<String> names = Image::GetIconLibraryNames("sysicons.icl");
for (int i = 0; i < names.Length(); i++) {
    Console::WriteLine(names[i]);
}

// Desktop integration
Desktop desktop(Color::Cyan);
desktop.LoadCursorFromLibrary(SystemIcons::LibraryPath, SystemIcons::CursorPointer);
desktop.AddIconFromLibrary(SystemIcons::LibraryPath, SystemIcons::Computer);
```

---

## TrueType Font Support (Complete)

### Overview

TrueType font (TTF) rendering using the stb_truetype single-header library. Supports loading TTF files and rendering text with configurable anti-aliasing.

### What Was Implemented

#### Font Class Extensions (`src/System/Drawing/Drawing.hpp/.cpp`)
- `Font::FromTrueType(path, pixelHeight, style)` - Load TTF font at specified size
- `Font::IsTrueType()` - Check if font is TTF or bitmap FON
- `Font::GetTTFInfo()` / `Font::GetTTFScale()` - Internal TTF rendering data

#### Direct TTF Rendering in DrawString
- Renders TTF glyphs directly without caching for correct positioning
- Uses `stbtt_GetCodepointHMetrics` for advance width and left side bearing
- Uses `stbtt_MakeCodepointBitmap` for glyph rasterization
- Sharp threshold rendering (gray > 128) for crisp text without blur

#### Third-Party Library
- `src/ThirdParty/stb_truetype.h` - Sean Barrett's public domain TTF rasterizer
- `src/ThirdParty/stb_truetype_impl.cpp` - Implementation file

### Key Technical Decisions

1. **Direct rendering vs glyph caching**: TTF glyphs are rendered directly in DrawString rather than pre-cached, because TTF positioning (left side bearing, baseline) requires rendering at the correct position.

2. **Sharp threshold vs anti-aliasing**: Using `gray > 128` threshold instead of alpha blending. Full anti-aliasing causes characters to blur together at small sizes. Sharp threshold gives crisp, bitmap-like edges.

3. **Font copy constructor**: Must re-initialize `stbtt_fontinfo` after copying bitmap data, as it contains internal pointers.

### Font Files

| File | Type | Notes |
|------|------|-------|
| TAHOMA.TTF | Sans Serif | Windows UI font, good screen legibility |
| TAHOMABD.TTF | Sans Serif Bold | Recommended for window titles |
| ROBOTO.TTF | Sans Serif | Google font, modern look |

### Usage Example

```cpp
// Load TrueType font
Font titleFont = Font::FromTrueType("C:\\TAHOMABD.TTF", 12);

// Use for window titles
window->SetFont(titleFont);

// Or draw directly
Graphics g(buffer);
g.DrawString("Hello World", titleFont, Color::White, 10, 10);
```

### QEMU Boot Configuration

For demos requiring large TTF files, use floppy boot with HDD data:
- **Floppy (A:)**: FreeDOS kernel, CWSDPMI, CTMOUSE
- **HDD (C:)**: Demo executable, TTF fonts, assets

See `qemu/scripts/run-gui-combo.sh` for the setup.

---

## FON Bitmap Font Format (Complete)

### Overview

Windows FON fonts use the NE (New Executable) format containing FNT font resources. Each FNT resource contains a header followed by a character table and glyph bitmap data.

### Glyph Bitmap Storage Format

**Critical**: FON glyph bitmaps are stored in **column-major** order by byte-columns, NOT row-major.

For a glyph with width W and height H:
- `pitch = ceil(W / 8)` = number of byte-columns
- Total bytes = `pitch * H`
- Storage: All H bytes of column 0, then all H bytes of column 1, etc.

**Byte access formula**: `src[byteCol * height + row]`

```cpp
// Correct (column-major):
Int32 byteCol = col / 8;
unsigned char byte = src[byteCol * height + row];

// WRONG (row-major) - produces garbled glyphs for chars > 8 pixels wide:
Int32 byteIndex = col / 8;
unsigned char byte = src[row * bytesPerRow + byteIndex];
```

### Why This Matters

For characters ≤8 pixels wide (single byte-column), both formulas give the same result. The bug only manifests for wider characters like 'W' (11 pixels), 'M', etc.

### FNT Header Structure

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0x00 | 2 | dfVersion | 0x0200 (V2) or 0x0300 (V3) |
| 0x02 | 4 | dfSize | Total resource size |
| 0x06 | 60 | dfCopyright | Copyright string |
| 0x42 | 2 | dfType | 0 = raster |
| 0x58 | 2 | dfPixHeight | Pixel height |
| 0x5f | 1 | dfFirstChar | First character code |
| 0x60 | 1 | dfLastChar | Last character code |
| ... | | | |

**Header sizes**:
- V2.0: 118 bytes
- V3.0: 148 bytes

**Character table** follows header:
- V2.0: 4 bytes per entry (2-byte width + 2-byte offset)
- V3.0: 6 bytes per entry (2-byte width + 4-byte offset)

### Reference Implementation

FreeType's `winfnt.c` (lines 1092-1106) transposes column-major source to row-major destination:
```c
// FreeType comment: "since glyphs are stored in columns and not in rows"
for ( ; pitch > 0; pitch--, column++ )
{
    for ( write = column; p < limit; p++, write += bitmap->pitch )
        *write = *p;
}
```

---

## PNG/JPEG Image Loading (Complete)

### Overview

Multi-format image loading using the stb_image single-header library. Supports PNG, JPEG, GIF, TGA, PSD formats with automatic format detection.

### What Was Implemented

#### Image Class Extensions (`src/System/Drawing/Drawing.hpp/.cpp`)
- `Image::FromFile(path)` - Auto-detect format and load (PNG, JPEG, GIF, TGA, PSD, BMP)
- `Image::FromPng(path)` - Load PNG image explicitly
- `Image::FromJpeg(path)` - Load JPEG image explicitly
- `Image::ScaleTo(width, height)` - Scale image using bilinear interpolation
- `Image::ScaleTo(Size)` - Scale using Size struct

#### Third-Party Library
- `src/ThirdParty/stb_image.h` - Sean Barrett's public domain image loader
- `src/ThirdParty/stb_image_impl.cpp` - Implementation file with STBI_NO_STDIO (uses our File I/O)

### Key Technical Decisions

1. **RGBA to ARGB conversion**: stb_image returns RGBA format, but WinDOS uses ARGB internally. Pixel data is converted during loading.

2. **Memory-based loading**: Uses `stbi_load_from_memory()` with our `File::ReadAllBytes()` since DJGPP stdio may have compatibility issues.

3. **Bilinear interpolation for scaling**: `ScaleTo()` uses fixed-point arithmetic (16.16 format) for accurate sub-pixel sampling with smooth results.

### Usage Example

```cpp
// Load image (auto-detects format)
Image splash = Image::FromFile("C:\\BOOT.PNG");

// Scale to screen size
Image fullscreen = splash.ScaleTo(800, 600);

// Draw to framebuffer
GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
fb->GetImage().CopyFrom(fullscreen, 0, 0);
GraphicsBuffer::FlushFrameBuffer();
```

### Boot Splash Screen

The forms demo displays a boot splash screen for 5 seconds before showing the desktop:
1. Loads `BOOT.PNG` or `BOOT.JPG` from C: drive (or current directory)
2. Scales image to fill the entire screen
3. Displays for 5 seconds using a busy-wait loop

---

## PENDING: IO/Devices Reorganization

### Status: APPROVED - Ready to Implement

A reorganization of `Platform/DOS` and `System/Devices` has been planned. See `./merge-plan.md` for full details.

### Problem

The current structure is confusing:
- `Platform/DOS/Graphics` vs `Platform/DOS/Video` vs `System/Devices/Display` - three names for related concepts
- `Platform/DOS/Keyboard` vs `System/Devices/Keyboard` - same name, different layers
- `Devices` separate from `IO`, but devices ARE I/O
- Two-layer abstraction adds complexity without benefit (this is DOS-only, forever)

### Current Structure (To Be Replaced)

```
src/
├── Platform/DOS/           # Low-level BIOS/port I/O
│   ├── Graphics.hpp/.cpp   # VGA/VBE modes
│   ├── Video.hpp/.cpp      # Text cursor
│   ├── Keyboard.hpp/.cpp   # INT 16h
│   ├── Mouse.hpp/.cpp      # INT 33h
│   └── System.hpp/.cpp     # INT 21h (Exit, etc.)
└── System/
    ├── Devices/Devices.hpp/.cpp  # Display, Mouse, Keyboard wrappers
    └── IO/IO.hpp/.cpp            # File class only
```

### Target Structure

```
src/System/IO/
├── Devices/
│   ├── Display.hpp/.cpp    # Merge: Graphics + Video + Display
│   ├── Keyboard.hpp/.cpp   # Merge: both Keyboard layers
│   └── Mouse.hpp/.cpp      # Merge: both Mouse layers
├── File.hpp/.cpp           # Rename from IO.hpp
└── Environment.hpp/.cpp    # New: Exit(), GetCurrentDirectory(), etc.
```

### Key Design Decisions

1. **Proper C++ encapsulation** - NO "Internal" namespaces (that's a code smell). Use:
   - Private static methods for BIOS calls
   - Anonymous namespaces in `.cpp` for internal structs/helpers

2. **Single namespace** - `System::IO::Devices` for all device classes

3. **System::Environment class** - New class for system interaction:
   ```cpp
   class Environment
   {
   public:
       static void Exit(Int32 exitCode);
       static String GetCommandLine();
       static String GetEnvironmentVariable(const String& name);
       static String GetCurrentDirectory();
       static void SetCurrentDirectory(const String& path);
       static String GetOSVersion();
       static const char* NewLine();  // "\r\n" for DOS
   };
   ```

### Class Design Pattern

Each device class follows this pattern:

```cpp
// Header: Public API only
namespace System::IO::Devices
{
    class Display
    {
    public:
        static Display GetCurrent();
        static void SetMode(DisplayMode mode);
        static void FadeIn(Int32 milliseconds);
        // ... other public methods

    private:
        // Low-level BIOS - truly hidden
        static bool DetectVBE();
        static bool SetVBEModeInternal(unsigned short mode);
        static void SetVGAModeInternal(unsigned char mode);
        // ... other private methods

        // Private state
        static Display _current;
        static bool _vbeAvailable;
    };
}
```

```cpp
// Implementation: Internal helpers in anonymous namespace
namespace System::IO::Devices
{
    namespace
    {
        // File-local structs - invisible outside this .cpp
        struct VbeInfoBlock { ... };
        struct VbeModeInfoBlock { ... };

        // File-local helpers
        bool CallVBEBios(int function, __dpmi_regs* regs) { ... }
    }

    // Public and private method implementations
    Display Display::GetCurrent() { ... }
}
```

### Migration Phases

1. **Create structure**: `mkdir -p src/System/IO/Devices`

2. **Mouse.hpp/.cpp** (simplest, start here)
   - Merge `Platform/DOS/Mouse` + `System/Devices::Mouse`
   - Private BIOS methods, public wrapper API

3. **Keyboard.hpp/.cpp**
   - Merge `Platform/DOS/Keyboard` + `System/Devices::Keyboard`

4. **Display.hpp/.cpp** (most complex)
   - Merge `Platform/DOS/Graphics` + `Platform/DOS/Video` + `System/Devices::Display`
   - VGA, VBE, text cursor, palette, gamma, LFB all in one class

5. **Environment.hpp/.cpp**
   - `Exit()`, `GetOSVersion()` from DOSSystem
   - Add `GetCurrentDirectory()`, `GetEnvironmentVariable()`
   - Inline `WriteString`/`ReadLine` into Console.cpp

6. **Rename IO.hpp → File.hpp**

7. **Update all includes** throughout codebase

8. **Update Makefile** with new source paths

9. **Delete old files**: `Platform/DOS/*`, `System/Devices/*`

10. **Test**: Build all, run test suite, run forms_demo

### Files to Create

| File | Content |
|------|---------|
| `src/System/IO/Devices/Display.hpp` | Display class (public API) |
| `src/System/IO/Devices/Display.cpp` | Display implementation |
| `src/System/IO/Devices/Keyboard.hpp` | Keyboard + KeyboardStatus classes |
| `src/System/IO/Devices/Keyboard.cpp` | Keyboard implementation |
| `src/System/IO/Devices/Mouse.hpp` | Mouse + MouseStatus classes |
| `src/System/IO/Devices/Mouse.cpp` | Mouse implementation |
| `src/System/IO/Environment.hpp` | Environment class |
| `src/System/IO/Environment.cpp` | Environment implementation |
| `src/System/IO/File.hpp` | Renamed from IO.hpp |
| `src/System/IO/File.cpp` | Renamed from IO.cpp |

### Files to Delete (After Migration)

- `src/Platform/DOS/*` (all files)
- `src/Platform/Platform.hpp`
- `src/System/Devices/*` (all files)
- `src/System/IO/IO.hpp` and `IO.cpp` (renamed to File.hpp/.cpp)

### Files to Update

| File | Changes |
|------|---------|
| `src/System/Drawing/Drawing.cpp` | `#include "System/IO/Devices/Display.hpp"` |
| `src/System/Windows/Forms/Forms.cpp` | Include all new device headers |
| `src/System/Console.cpp` | Use new Keyboard, inline DOS writes |
| `src/rtcorlib.hpp` | Update master includes |
| `Makefile` | Update SOURCES list |

### Namespace Changes

| Old | New |
|-----|-----|
| `Platform::DOS::Graphics` | `System::IO::Devices::Display` (private methods) |
| `Platform::DOS::Video` | `System::IO::Devices::Display` (private methods) |
| `Platform::DOS::Keyboard` | `System::IO::Devices::Keyboard` (private methods) |
| `Platform::DOS::Mouse` | `System::IO::Devices::Mouse` (private methods) |
| `Platform::DOS::DOSSystem` | `System::IO::Environment` + Console.cpp |
| `System::Devices::Display` | `System::IO::Devices::Display` |
| `System::Devices::Mouse` | `System::IO::Devices::Mouse` |
| `System::Devices::Keyboard` | `System::IO::Devices::Keyboard` |
| `System::IO::File` | `System::IO::File` (unchanged, just renamed file) |

### Reference

See `./merge-plan.md` for:
- Detailed class APIs with all methods
- Full rationale for decisions
- Estimated effort (~6.5 hours)

---

## See Also

- `WinDOS.md` - Full API documentation (MSDN-style)
- `merge-plan.md` - IO/Devices reorganization plan
- `tests/*.cpp` - Usage examples
