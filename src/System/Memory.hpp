#ifndef SYSTEM_MEMORY_HPP
#define SYSTEM_MEMORY_HPP

#include "Types.hpp"
#include "String.hpp"
#include <cstddef>

namespace System {

/******************************************************************************/
/*    System::MemoryPool                                                       */
/******************************************************************************/

/**
 * A simple fixed-size block memory pool for efficient small allocations.
 *
 * Use MemoryPool when allocating many objects of the same size to reduce
 * heap fragmentation and allocation overhead. Common uses:
 * - GUI control structures
 * - Event objects
 * - Node-based data structures
 *
 * Example:
 *   MemoryPool pool(sizeof(MyObject), 100);  // 100 blocks
 *   void* ptr = pool.Allocate();
 *   MyObject* obj = new(ptr) MyObject();     // Placement new
 *   obj->~MyObject();                        // Manual destructor
 *   pool.Free(ptr);                          // Return to pool
 */
class MemoryPool {
private:
    struct Block {
        Block* next;
    };

    unsigned char* _memory;   // Raw memory buffer
    Block* _freeList;         // Head of free block list
    int _blockSize;           // Size of each block (>= sizeof(Block*))
    int _blockCount;          // Total number of blocks
    int _freeCount;           // Number of free blocks

public:
    /**
     * Creates a memory pool with the specified block size and count.
     * @param blockSize Size of each allocation block (minimum 8 bytes)
     * @param blockCount Number of blocks to pre-allocate
     */
    MemoryPool(Int32 blockSize, Int32 blockCount);

    /**
     * Destroys the memory pool and frees all memory.
     * Warning: All allocated blocks become invalid.
     */
    ~MemoryPool();

    // Non-copyable
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    // Movable
    MemoryPool(MemoryPool&& other) noexcept;
    MemoryPool& operator=(MemoryPool&& other) noexcept;

    /**
     * Allocates a block from the pool.
     * @return Pointer to allocated block, or nullptr if pool is exhausted.
     */
    void* Allocate();

    /**
     * Returns a block to the pool.
     * @param ptr Pointer previously returned by Allocate().
     *            Passing nullptr is safe (no-op).
     *            Passing invalid pointer is undefined behavior.
     */
    void Free(void* ptr);

    /**
     * Resets the pool, making all blocks available again.
     * Warning: All previously allocated blocks become invalid.
     */
    void Reset();

    // Properties
    Int32 BlockSize() const { return Int32(_blockSize); }
    Int32 BlockCount() const { return Int32(_blockCount); }
    Int32 FreeCount() const { return Int32(_freeCount); }
    Int32 UsedCount() const { return Int32(_blockCount - _freeCount); }
    Boolean IsEmpty() const { return Boolean(_freeCount == 0); }
    Boolean IsFull() const { return Boolean(_freeCount == _blockCount); }
};

/******************************************************************************/
/*    System::StringIntern                                                     */
/******************************************************************************/

/**
 * Provides string interning for memory-efficient string storage.
 *
 * String interning ensures that equal strings share the same memory,
 * reducing memory usage when many duplicate strings exist. Interned
 * strings can be compared by pointer (O(1)) instead of by value (O(n)).
 *
 * Common interned strings (True, False, etc.) are pre-initialized.
 *
 * Example:
 *   const char* s1 = StringIntern::Intern("Hello");
 *   const char* s2 = StringIntern::Intern("Hello");
 *   // s1 == s2 (same pointer, not just equal content)
 *
 * Note: Interned strings live for the duration of the program.
 *       Do not intern dynamically generated strings in loops.
 */
class StringIntern {
private:
    // Simple hash table entry
    struct Entry {
        char* str;
        int length;
        unsigned int hash;
        Entry* next;

        Entry() : str(nullptr), length(0), hash(0), next(nullptr) {}
    };

    static const int TABLE_SIZE = 127;  // Prime number for better distribution
    static Entry* _table[TABLE_SIZE];
    static bool _initialized;

    static unsigned int Hash(const char* str, int length);
    static void Initialize();

public:
    /**
     * Interns a string, returning a pointer to the canonical version.
     * If the string is already interned, returns the existing pointer.
     * If not, creates a copy and stores it in the intern pool.
     *
     * @param str The string to intern (null-terminated)
     * @return Pointer to the interned string (never null for non-null input)
     */
    static const char* Intern(const char* str);

    /**
     * Interns a string with explicit length.
     * @param str The string to intern (may not be null-terminated)
     * @param length The length of the string
     * @return Pointer to the interned string
     */
    static const char* Intern(const char* str, Int32 length);

    /**
     * Interns a String object.
     * @param str The String to intern
     * @return Pointer to the interned C string
     */
    static const char* Intern(const String& str);

    /**
     * Checks if a string is already interned.
     * @param str The string to check
     * @return true if the string is in the intern pool
     */
    static Boolean IsInterned(const char* str);

    /**
     * Gets the number of unique strings in the intern pool.
     */
    static Int32 Count();

    // Pre-interned common strings (for fast access)
    static const char* True()  { Initialize(); return Intern("True"); }
    static const char* False() { Initialize(); return Intern("False"); }
    static const char* Empty() { Initialize(); return Intern(""); }
    static const char* Null()  { Initialize(); return Intern("null"); }
    static const char* NewLine() { Initialize(); return Intern("\n"); }

private:
    StringIntern() = delete;  // Static class
};

} // namespace System

#endif // SYSTEM_MEMORY_HPP
