#ifndef SYSTEM_MEMORY_HPP
#define SYSTEM_MEMORY_HPP

#include "Types.hpp"
#include "String.hpp"
#include <cstddef>

namespace System
{

/******************************************************************************/
/*    System::MemoryPool                                                       */
/******************************************************************************/

/// @brief A fixed-size block memory pool for efficient small object allocations.
///
/// MemoryPool provides fast O(1) allocation and deallocation of fixed-size
/// memory blocks, reducing heap fragmentation and allocation overhead.
/// Internally uses a free-list to track available blocks.
///
/// @warning All allocated blocks become invalid when the pool is destroyed
/// or Reset() is called. Ensure objects are properly destructed before
/// freeing their memory.
///
/// Common use cases:
/// - GUI control structures
/// - Event objects
/// - Node-based data structures (linked lists, trees)
///
/// Example:
/// @code
///   MemoryPool pool(sizeof(MyObject), 100);  // 100 blocks
///   void* ptr = pool.Allocate();
///   MyObject* obj = new(ptr) MyObject();     // Placement new
///   obj->~MyObject();                        // Manual destructor
///   pool.Free(ptr);                          // Return to pool
/// @endcode
class MemoryPool
{
    /// @brief Internal free-list node structure.
    struct Block
    {
        Block* next;
    };

    unsigned char* _memory;   ///< Raw memory buffer containing all blocks.
    Block* _freeList;         ///< Head of the free block linked list.
    Int32 _blockSize;         ///< Size of each block in bytes (>= sizeof(Block*)).
    Int32 _blockCount;        ///< Total number of blocks in the pool.
    Int32 _freeCount;         ///< Current number of available (free) blocks.

public:
    /// @brief Creates a memory pool with the specified block size and count.
    /// @param blockSize Size of each allocation block in bytes.
    ///        Minimum is sizeof(void*) (8 bytes on 32-bit systems).
    ///        Smaller values are automatically rounded up.
    /// @param blockCount Number of blocks to pre-allocate.
    ///        Must be greater than 0.
    /// @warning Allocates blockSize * blockCount bytes immediately.
    MemoryPool(Int32 blockSize, Int32 blockCount);

    /// @brief Destroys the memory pool and frees all underlying memory.
    /// @warning All blocks previously returned by Allocate() become invalid.
    ///          Ensure all objects have been destructed before destroying the pool.
    ~MemoryPool();

    // Non-copyable
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    /// @brief Move constructor. Transfers ownership of the memory pool.
    /// @param other The pool to move from. Left in an empty state.
    MemoryPool(MemoryPool&& other) noexcept;

    /// @brief Move assignment operator. Transfers ownership of the memory pool.
    /// @param other The pool to move from. Left in an empty state.
    /// @return Reference to this pool.
    MemoryPool& operator=(MemoryPool&& other) noexcept;

    /// @brief Allocates a block from the pool.
    /// @return Pointer to an uninitialized memory block of BlockSize() bytes,
    ///         or nullptr if the pool is exhausted (no free blocks available).
    /// @warning The returned memory is uninitialized. Use placement new to
    ///          construct objects.
    void* Allocate();

    /// @brief Returns a previously allocated block to the pool.
    /// @param ptr Pointer previously returned by Allocate() on this pool.
    /// @warning Passing nullptr is safe (no-op).
    /// @warning Passing a pointer not from this pool is undefined behavior.
    /// @warning Double-freeing the same pointer is undefined behavior.
    /// @warning The caller must destruct any object in the block before freeing.
    void Free(void* ptr);

    /// @brief Resets the pool, making all blocks available again.
    /// @warning All previously allocated blocks become invalid immediately.
    ///          Ensure all objects have been destructed before calling Reset().
    ///          This is faster than freeing blocks individually.
    void Reset();

    /// @brief Gets the size of each block in bytes.
    /// @return The block size specified at construction (possibly rounded up).
    Int32 BlockSize() const
    {
        return _blockSize;
    }

    /// @brief Gets the total number of blocks in the pool.
    /// @return The block count specified at construction.
    Int32 BlockCount() const
    {
        return _blockCount;
    }

    /// @brief Gets the current number of free (unallocated) blocks.
    /// @return Number of blocks available for allocation.
    Int32 FreeCount() const
    {
        return _freeCount;
    }

    /// @brief Gets the current number of allocated (in-use) blocks.
    /// @return BlockCount() - FreeCount().
    Int32 UsedCount() const
    {
        return _blockCount - _freeCount;
    }

    /// @brief Checks if the pool has no free blocks remaining.
    /// @return true if FreeCount() == 0, meaning Allocate() will return nullptr.
    Boolean IsEmpty() const
    {
        return Boolean(_freeCount == Int32(0));
    }

    /// @brief Checks if all blocks in the pool are free.
    /// @return true if FreeCount() == BlockCount(), meaning no blocks are allocated.
    Boolean IsFull() const
    {
        return Boolean(_freeCount == _blockCount);
    }
};

/******************************************************************************/
/*    System::StringIntern                                                     */
/******************************************************************************/

/// @brief Provides string interning for memory-efficient string storage.
///
/// String interning ensures that equal strings share the same memory address,
/// reducing memory usage when many duplicate strings exist. Interned strings
/// can be compared by pointer equality O(1) instead of by content O(n).
///
/// The intern pool uses a hash table for O(1) average lookup time.
/// Common strings (True, False, Empty, etc.) are pre-initialized on first use.
///
/// @warning Interned strings are never freed and live for the program's duration.
///          Do not intern dynamically generated or temporary strings in loops,
///          as this will cause unbounded memory growth.
///
/// @warning The StringIntern class is not thread-safe. All calls must be made
///          from the same thread, or external synchronization must be provided.
///
/// Example:
/// @code
///   const char* s1 = StringIntern::Intern("Hello");
///   const char* s2 = StringIntern::Intern("Hello");
///   // s1 == s2 (same pointer, not just equal content)
///   // Fast comparison: if (s1 == s2) instead of strcmp(s1, s2)
/// @endcode
class StringIntern
{
    /// @brief Hash table entry for storing interned strings.
    struct Entry
    {
        char* str;           ///< The interned string (owned, heap-allocated).
        int length;          ///< Length of the string (cached).
        unsigned int hash;   ///< Hash value (cached for resize operations).
        Entry* next;         ///< Next entry in the collision chain.

        /// @brief Default constructor.
        Entry() : str(nullptr), length(0), hash(0), next(nullptr)
        {
        }
    };

    static const int TABLE_SIZE = 127;  ///< Hash table size (prime for better distribution).
    static Entry* _table[TABLE_SIZE];   ///< Hash table buckets.
    static Boolean _initialized;        ///< Whether the pool has been initialized.

    /// @brief Computes a hash value for a string.
    /// @param str The string to hash.
    /// @param length The length of the string.
    /// @return A 32-bit hash value.
    static UInt32 Hash(const char* str, Int32 length);

    /// @brief Initializes the intern pool (called automatically on first use).
    static void Initialize();

public:
    /// @brief Interns a null-terminated string.
    ///
    /// If the string is already interned, returns the existing canonical pointer.
    /// If not, creates a copy and stores it in the intern pool.
    ///
    /// @param str The null-terminated string to intern.
    /// @return Pointer to the canonical interned string.
    ///         Returns nullptr only if str is nullptr.
    /// @warning The returned pointer is valid for the lifetime of the program.
    ///          Do not free or modify the returned string.
    static const char* Intern(const char* str);

    /// @brief Interns a string with explicit length.
    ///
    /// Useful for interning substrings or strings that may not be null-terminated.
    ///
    /// @param str The string to intern (does not need to be null-terminated).
    /// @param length The number of characters to intern.
    /// @return Pointer to the canonical interned string (null-terminated).
    /// @warning The returned pointer is valid for the lifetime of the program.
    static const char* Intern(const char* str, Int32 length);

    /// @brief Interns a String object.
    /// @param str The String to intern.
    /// @return Pointer to the canonical interned C string.
    /// @warning The returned pointer is valid for the lifetime of the program.
    static const char* Intern(const String& str);

    /// @brief Checks if a string is already in the intern pool.
    /// @param str The null-terminated string to check.
    /// @return true if the exact string content exists in the pool,
    ///         false otherwise.
    static Boolean IsInterned(const char* str);

    /// @brief Gets the number of unique strings in the intern pool.
    /// @return The count of interned strings.
    static Int32 Count();

    /// @brief Gets the interned string "True".
    /// @return Canonical pointer to "True".
    static const char* True()
    {
        Initialize();
        return Intern("True");
    }

    /// @brief Gets the interned string "False".
    /// @return Canonical pointer to "False".
    static const char* False()
    {
        Initialize();
        return Intern("False");
    }

    /// @brief Gets the interned empty string "".
    /// @return Canonical pointer to "".
    static const char* Empty()
    {
        Initialize();
        return Intern("");
    }

    /// @brief Gets the interned string "null".
    /// @return Canonical pointer to "null".
    static const char* Null()
    {
        Initialize();
        return Intern("null");
    }

    /// @brief Gets the interned newline string "\n".
    /// @return Canonical pointer to "\n".
    static const char* NewLine()
    {
        Initialize();
        return Intern("\n");
    }

private:
    StringIntern() = delete;  ///< Static class - cannot be instantiated.
};

} // namespace System

#endif // SYSTEM_MEMORY_HPP
