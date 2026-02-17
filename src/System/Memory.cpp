#include "Memory.hpp"
#include "Exception.hpp"
#include <cstring>
#include <cstdlib>

namespace System {

// ============================================================================
// MemoryPool Implementation
// ============================================================================

MemoryPool::MemoryPool(Int32 blockSize, Int32 blockCount)
    : _memory(nullptr)
    , _freeList(nullptr)
    , _blockSize(0)
    , _blockCount(0)
    , _freeCount(0) {

    int size = static_cast<int>(blockSize);
    int count = static_cast<int>(blockCount);

    if (size < 1) {
        throw ArgumentOutOfRangeException("blockSize", "Block size must be positive.");
    }
    if (count < 1) {
        throw ArgumentOutOfRangeException("blockCount", "Block count must be positive.");
    }

    // Ensure block size is at least large enough for a pointer (for free list)
    const int minSize = static_cast<int>(sizeof(Block));
    _blockSize = (size < minSize) ? minSize : size;
    _blockCount = count;

    // Allocate raw memory
    size_t totalSize = static_cast<size_t>(_blockSize) * static_cast<size_t>(_blockCount);
    _memory = static_cast<unsigned char*>(std::malloc(totalSize));
    if (!_memory) {
        throw InvalidOperationException("Failed to allocate memory pool.");
    }

    // Initialize free list
    Reset();
}

MemoryPool::~MemoryPool() {
    std::free(_memory);
    _memory = nullptr;
    _freeList = nullptr;
    _blockSize = 0;
    _blockCount = 0;
    _freeCount = 0;
}

MemoryPool::MemoryPool(MemoryPool&& other) noexcept
    : _memory(other._memory)
    , _freeList(other._freeList)
    , _blockSize(other._blockSize)
    , _blockCount(other._blockCount)
    , _freeCount(other._freeCount) {
    other._memory = nullptr;
    other._freeList = nullptr;
    other._blockSize = 0;
    other._blockCount = 0;
    other._freeCount = 0;
}

MemoryPool& MemoryPool::operator=(MemoryPool&& other) noexcept {
    if (this != &other) {
        std::free(_memory);

        _memory = other._memory;
        _freeList = other._freeList;
        _blockSize = other._blockSize;
        _blockCount = other._blockCount;
        _freeCount = other._freeCount;

        other._memory = nullptr;
        other._freeList = nullptr;
        other._blockSize = 0;
        other._blockCount = 0;
        other._freeCount = 0;
    }
    return *this;
}

void* MemoryPool::Allocate() {
    if (!_freeList) {
        return nullptr;  // Pool exhausted
    }

    // Pop from free list
    Block* block = _freeList;
    _freeList = block->next;
    _freeCount--;

    return static_cast<void*>(block);
}

void MemoryPool::Free(void* ptr) {
    if (!ptr) return;

    // Push onto free list
    Block* block = static_cast<Block*>(ptr);
    block->next = _freeList;
    _freeList = block;
    _freeCount++;
}

void MemoryPool::Reset() {
    // Rebuild free list linking all blocks
    _freeList = nullptr;
    unsigned char* current = _memory;

    for (int i = 0; i < _blockCount; i++) {
        Block* block = reinterpret_cast<Block*>(current);
        block->next = _freeList;
        _freeList = block;
        current += _blockSize;
    }

    _freeCount = _blockCount;
}

// ============================================================================
// StringIntern Implementation
// ============================================================================

// Static member initialization
StringIntern::Entry* StringIntern::_table[TABLE_SIZE] = { nullptr };
bool StringIntern::_initialized = false;

void StringIntern::Initialize() {
    if (_initialized) return;
    _initialized = true;

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

unsigned int StringIntern::Hash(const char* str, int length) {
    // FNV-1a hash
    unsigned int hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= static_cast<unsigned char>(str[i]);
        hash *= 16777619u;
    }
    return hash;
}

const char* StringIntern::Intern(const char* str) {
    if (!str) return nullptr;
    return Intern(str, Int32(static_cast<int>(std::strlen(str))));
}

const char* StringIntern::Intern(const char* str, Int32 length) {
    Initialize();

    if (!str) return nullptr;

    int len = static_cast<int>(length);
    if (len < 0) len = 0;

    unsigned int hash = Hash(str, len);
    int bucket = static_cast<int>(hash % TABLE_SIZE);

    // Search for existing entry
    Entry* entry = _table[bucket];
    while (entry) {
        if (entry->hash == hash &&
            entry->length == len &&
            (len == 0 || std::memcmp(entry->str, str, len) == 0)) {
            // Found existing interned string
            return entry->str;
        }
        entry = entry->next;
    }

    // Not found - create new entry
    Entry* newEntry = new Entry();
    newEntry->hash = hash;
    newEntry->length = len;
    newEntry->str = new char[len + 1];
    if (len > 0) {
        std::memcpy(newEntry->str, str, len);
    }
    newEntry->str[len] = '\0';

    // Insert at head of bucket
    newEntry->next = _table[bucket];
    _table[bucket] = newEntry;

    return newEntry->str;
}

const char* StringIntern::Intern(const String& str) {
    return Intern(str.CStr(), str.Length());
}

Boolean StringIntern::IsInterned(const char* str) {
    Initialize();

    if (!str) return Boolean(false);

    int len = static_cast<int>(std::strlen(str));
    unsigned int hash = Hash(str, len);
    int bucket = static_cast<int>(hash % TABLE_SIZE);

    Entry* entry = _table[bucket];
    while (entry) {
        // Check if this is the SAME pointer (interned strings share memory)
        if (entry->str == str) {
            return Boolean(true);
        }
        // Also check by value in case user is checking a different pointer
        if (entry->hash == hash &&
            entry->length == len &&
            std::memcmp(entry->str, str, len) == 0) {
            return Boolean(true);
        }
        entry = entry->next;
    }

    return Boolean(false);
}

Int32 StringIntern::Count() {
    Initialize();

    int count = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry* entry = _table[i];
        while (entry) {
            count++;
            entry = entry->next;
        }
    }
    return Int32(count);
}

} // namespace System
