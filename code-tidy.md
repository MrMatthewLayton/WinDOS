# Code Tidy Analysis

Generated: 2026-02-19

## clang-tidy Status

**clang-tidy is not available on this system.**

To install clang-tidy on macOS:

```bash
# Install via Homebrew
brew install llvm

# Add to PATH (add to ~/.zshrc or ~/.bashrc)
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"

# Verify installation
clang-tidy --version
```

---

## Alternative: Manual Code Quality Checks

Without clang-tidy, here are the key areas that should be reviewed manually:

### 1. Modernization Opportunities

| Check | Description | Priority |
|-------|-------------|----------|
| Use `nullptr` | Replace `NULL` and `0` for pointers | High |
| Use `auto` | Type deduction where appropriate | Medium |
| Range-based for loops | Replace index-based iteration | Medium |
| `override` keyword | Ensure all overrides are marked | High |
| `[[nodiscard]]` | Mark functions whose return shouldn't be ignored | Medium |

### 2. Potential Bug Patterns

| Check | Description | Priority |
|-------|-------------|----------|
| Uninitialized members | Ensure all class members initialized | High |
| Resource leaks | RAII usage, delete in destructors | High |
| Integer overflow | Especially in size calculations | Medium |
| Null pointer dereference | Check pointers before use | High |

### 3. Performance Improvements

| Check | Description | Priority |
|-------|-------------|----------|
| Pass by const reference | For non-trivial types | High |
| Move semantics | Use `std::move` where appropriate | Medium |
| Reserve container capacity | Pre-allocate when size known | Low |
| Inline small functions | Header-only for tiny functions | Low |

### 4. Style and Consistency

| Check | Description | Priority |
|-------|-------------|----------|
| Const correctness | Mark methods and params const | High |
| Explicit constructors | Single-arg constructors explicit | High |
| Naming conventions | Consistent member/method naming | Medium |

---

## Recommended clang-tidy Configuration

Once clang-tidy is installed, create `.clang-tidy` in project root:

```yaml
---
Checks: >
  -*,
  bugprone-*,
  -bugprone-easily-swappable-parameters,
  cppcoreguidelines-*,
  -cppcoreguidelines-avoid-magic-numbers,
  -cppcoreguidelines-pro-type-reinterpret-cast,
  -cppcoreguidelines-pro-bounds-pointer-arithmetic,
  -cppcoreguidelines-owning-memory,
  misc-*,
  -misc-non-private-member-variables-in-classes,
  modernize-*,
  -modernize-use-trailing-return-type,
  performance-*,
  readability-*,
  -readability-magic-numbers,
  -readability-identifier-length

WarningsAsErrors: ''

HeaderFilterRegex: '.*'

CheckOptions:
  - key: readability-identifier-naming.ClassCase
    value: CamelCase
  - key: readability-identifier-naming.MethodCase
    value: CamelCase
  - key: readability-identifier-naming.PrivateMemberPrefix
    value: '_'
  - key: readability-identifier-naming.ConstantCase
    value: CamelCase
  - key: modernize-use-override.IgnoreDestructors
    value: true
```

---

## Running clang-tidy

Once installed and configured:

```bash
# Generate compile_commands.json (needed for clang-tidy)
# For Makefile projects, use Bear:
brew install bear
bear -- make clean all

# Run clang-tidy on a single file
clang-tidy src/System/String.cpp

# Run on all source files
find src -name "*.cpp" | xargs -I{} clang-tidy {} -- -std=c++17 -I./src

# Run with fixes (careful - modifies files)
clang-tidy --fix src/System/String.cpp
```

---

## Known Issues to Address

Based on manual code review:

### High Priority

1. **Virtual destructors**: Ensure all base classes with virtual methods have virtual destructors
   - `Control` class: Has virtual destructor ✓
   - Check all others

2. **Override keyword**: Add `override` to all virtual method overrides
   - Many methods in Forms.hpp/cpp missing explicit `override`

3. **Const correctness**: Several getters could be `const`
   - Check throughout codebase

### Medium Priority

4. **Magic numbers**: Use named constants
   - `ICON_SIZE`, `TITLE_BAR_HEIGHT` exist but others are inline

5. **Explicit constructors**: Single-arg constructors should be explicit
   - `Color(unsigned int argb)` - should be explicit
   - `Point(...)` - check
   - `Rectangle(...)` - check

6. **Pass by const reference**: Some methods pass `String` by value
   - Should pass by `const String&`

### Low Priority

7. **auto keyword usage**: Could reduce verbosity in some places
8. **Range-based for loops**: Some index-based loops could be simplified

---

## Next Steps

1. Install clang-tidy via Homebrew
2. Create `.clang-tidy` configuration file
3. Generate `compile_commands.json` with Bear
4. Run clang-tidy on each source file
5. Address findings in priority order
6. Add clang-tidy to CI/build process
