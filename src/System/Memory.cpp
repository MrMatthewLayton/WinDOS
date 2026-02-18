#include "Memory.hpp"
#include "Exception.hpp"
#include <cstring>
#include <cstdlib>

namespace System
{

// ============================================================================
// MemoryPool Implementation
// ============================================================================

MemoryPool::MemoryPool(Int32 blockSize, Int32 blockCount)
    : _memory(nullptr)
    , _freeList(nullptr)
    , _blockSize(Int32(0))
    , _blockCount(Int32(0))
    , _freeCount(Int32(0))
{

    Int32 size = blockSize;
    Int32 count = blockCount;

    if (size < Int32(1))
    {
        throw ArgumentOutOfRangeException("blockSize", "Block size must be positive.");
    }
    if (count < Int32(1))
    {
        throw ArgumentOutOfRangeException("blockCount", "Block count must be positive.");
    }

    // Ensure block size is at least large enough for a pointer (for free list)
    const Int32 minSize = Int32(static_cast<int>(sizeof(Block)));
    _blockSize = (size < minSize) ? minSize : size;
    _blockCount = count;

    // Allocate raw memory
    size_t totalSize = static_cast<size_t>(_blockSize) * static_cast<size_t>(_blockCount);
    _memory = static_cast<unsigned char*>(std::malloc(totalSize));
    if (!_memory)
    {
        throw InvalidOperationException("Failed to allocate memory pool.");
    }

    // Initialize free list
    Reset();
}

MemoryPool::~MemoryPool()
{
    std::free(_memory);
    _memory = nullptr;
    _freeList = nullptr;
    _blockSize = Int32(0);
    _blockCount = Int32(0);
    _freeCount = Int32(0);
}

MemoryPool::MemoryPool(MemoryPool&& other) noexcept
    : _memory(other._memory)
    , _freeList(other._freeList)
    , _blockSize(other._blockSize)
    , _blockCount(other._blockCount)
    , _freeCount(other._freeCount)
{
    other._memory = nullptr;
    other._freeList = nullptr;
    other._blockSize = Int32(0);
    other._blockCount = Int32(0);
    other._freeCount = Int32(0);
}

MemoryPool& MemoryPool::operator=(MemoryPool&& other) noexcept
{
    if (this != &other)
    {
        std::free(_memory);

        _memory = other._memory;
        _freeList = other._freeList;
        _blockSize = other._blockSize;
        _blockCount = other._blockCount;
        _freeCount = other._freeCount;

        other._memory = nullptr;
        other._freeList = nullptr;
        other._blockSize = Int32(0);
        other._blockCount = Int32(0);
        other._freeCount = Int32(0);
    }
    return *this;
}

void* MemoryPool::Allocate()
{
    if (!_freeList)
    {
        return nullptr;  // Pool exhausted
    }

    // Pop from free list
    Block* block = _freeList;
    _freeList = block->next;
    _freeCount -= Int32(1);

    return static_cast<void*>(block);
}

void MemoryPool::Free(void* ptr)
{
    if (!ptr) return;

    // Push onto free list
    Block* block = static_cast<Block*>(ptr);
    block->next = _freeList;
    _freeList = block;
    _freeCount += Int32(1);
}

void MemoryPool::Reset()
{
    // Rebuild free list linking all blocks
    _freeList = nullptr;
    unsigned char* current = _memory;

    for (Int32 i = Int32(0); i < _blockCount; i += Int32(1))
    {
        Block* block = reinterpret_cast<Block*>(current);
        block->next = _freeList;
        _freeList = block;
        current += static_cast<int>(_blockSize);
    }

    _freeCount = _blockCount;
}

// ============================================================================
// StringIntern Implementation
// ============================================================================

// Static member initialization
StringIntern::Entry* StringIntern::_table[TABLE_SIZE] = { nullptr };
Boolean StringIntern::_initialized = Boolean(false);

void StringIntern::Initialize()
{
    if (static_cast<bool>(_initialized)) return;
    _initialized = Boolean(true);

    // Pre-intern common strings
    Intern("");
    Intern("True");
    Intern("False");
    Intern("null");
    Intern("\n");
    Intern(" ");
    Intern("0");
    Intern("1");
    Intern("-1");
}

UInt32 StringIntern::Hash(const char* str, Int32 length)
{
    // FNV-1a hash
    UInt32 hash = UInt32(2166136261u);
    for (Int32 i = Int32(0); i < length; i += Int32(1))
    {
        hash ^= UInt32(static_cast<unsigned char>(str[static_cast<int>(i)]));
        hash *= UInt32(16777619u);
    }
    return hash;
}

const char* StringIntern::Intern(const char* str)
{
    if (!str) return nullptr;
    return Intern(str, Int32(static_cast<int>(std::strlen(str))));
}

const char* StringIntern::Intern(const char* str, Int32 length)
{
    Initialize();

    if (!str) return nullptr;

    Int32 len = length;
    if (len < Int32(0)) len = Int32(0);

    UInt32 hash = Hash(str, len);
    Int32 bucket = Int32(static_cast<int>(static_cast<unsigned int>(hash) % TABLE_SIZE));

    // Search for existing entry
    Entry* entry = _table[static_cast<int>(bucket)];
    while (entry)
    {
        if (entry->hash == static_cast<unsigned int>(hash) &&
            entry->length == static_cast<int>(len) &&
            (static_cast<int>(len) == 0 || std::memcmp(entry->str, str, static_cast<int>(len)) == 0))
        {
            // Found existing interned string
            return entry->str;
        }
        entry = entry->next;
    }

    // Not found - create new entry
    Entry* newEntry = new Entry();
    newEntry->hash = static_cast<unsigned int>(hash);
    newEntry->length = static_cast<int>(len);
    newEntry->str = new char[static_cast<int>(len) + 1];
    if (len > Int32(0))
    {
        std::memcpy(newEntry->str, str, static_cast<int>(len));
    }
    newEntry->str[static_cast<int>(len)] = '\0';

    // Insert at head of bucket
    newEntry->next = _table[static_cast<int>(bucket)];
    _table[static_cast<int>(bucket)] = newEntry;

    return newEntry->str;
}

const char* StringIntern::Intern(const String& str)
{
    return Intern(str.GetRawString(), str.Length());
}

Boolean StringIntern::IsInterned(const char* str)
{
    Initialize();

    if (!str) return Boolean(false);

    Int32 len = Int32(static_cast<int>(std::strlen(str)));
    UInt32 hash = Hash(str, len);
    Int32 bucket = Int32(static_cast<int>(static_cast<unsigned int>(hash) % TABLE_SIZE));

    Entry* entry = _table[static_cast<int>(bucket)];
    while (entry)
    {
        // Check if this is the SAME pointer (interned strings share memory)
        if (entry->str == str)
        {
            return Boolean(true);
        }
        // Also check by value in case user is checking a different pointer
        if (entry->hash == static_cast<unsigned int>(hash) &&
            entry->length == static_cast<int>(len) &&
            std::memcmp(entry->str, str, static_cast<int>(len)) == 0)
        {
            return Boolean(true);
        }
        entry = entry->next;
    }

    return Boolean(false);
}

Int32 StringIntern::Count()
{
    Initialize();

    Int32 count = Int32(0);
    for (Int32 i = Int32(0); i < Int32(TABLE_SIZE); i += Int32(1))
    {
        Entry* entry = _table[static_cast<int>(i)];
        while (entry)
        {
            count += Int32(1);
            entry = entry->next;
        }
    }
    return count;
}

} // namespace System
