// Native test for Memory classes (runs on host system, not DOS)
// This validates MemoryPool and StringIntern logic
// Self-contained implementation to avoid rtcorlib dependencies

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstddef>

int passed = 0;
int failed = 0;

#define TEST(name) std::cout << "  " << name << "... "
#define PASS() do { std::cout << "PASS\n"; passed++; } while(0)
#define FAIL(msg) do { std::cout << "FAIL: " << msg << "\n"; failed++; } while(0)

// ============================================================================
// Minimal MemoryPool Implementation (matches bcl/System/Memory.cpp)
// ============================================================================

class MemoryPool {
private:
    struct Block {
        Block* next;
    };

    unsigned char* _memory;
    Block* _freeList;
    int _blockSize;
    int _blockCount;
    int _freeCount;

public:
    MemoryPool(int blockSize, int blockCount)
        : _memory(nullptr)
        , _freeList(nullptr)
        , _blockSize(0)
        , _blockCount(0)
        , _freeCount(0) {

        if (blockSize < 1 || blockCount < 1) {
            throw std::runtime_error("Invalid size");
        }

        const int minSize = static_cast<int>(sizeof(Block));
        _blockSize = (blockSize < minSize) ? minSize : blockSize;
        _blockCount = blockCount;

        size_t totalSize = static_cast<size_t>(_blockSize) * static_cast<size_t>(_blockCount);
        _memory = static_cast<unsigned char*>(std::malloc(totalSize));
        if (!_memory) {
            throw std::runtime_error("Allocation failed");
        }

        Reset();
    }

    ~MemoryPool() {
        std::free(_memory);
    }

    MemoryPool(MemoryPool&& other) noexcept
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

    void* Allocate() {
        if (!_freeList) return nullptr;
        Block* block = _freeList;
        _freeList = block->next;
        _freeCount--;
        return static_cast<void*>(block);
    }

    void Free(void* ptr) {
        if (!ptr) return;
        Block* block = static_cast<Block*>(ptr);
        block->next = _freeList;
        _freeList = block;
        _freeCount++;
    }

    void Reset() {
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

    int BlockCount() const { return _blockCount; }
    int FreeCount() const { return _freeCount; }
    int UsedCount() const { return _blockCount - _freeCount; }
    bool IsEmpty() const { return _freeCount == 0; }
    bool IsFull() const { return _freeCount == _blockCount; }
};

// ============================================================================
// Minimal StringIntern Implementation (matches bcl/System/Memory.cpp)
// ============================================================================

class StringIntern {
private:
    struct Entry {
        char* str;
        int length;
        unsigned int hash;
        Entry* next;
        Entry() : str(nullptr), length(0), hash(0), next(nullptr) {}
    };

    static const int TABLE_SIZE = 127;
    static Entry* _table[TABLE_SIZE];
    static bool _initialized;

    static unsigned int Hash(const char* str, int length) {
        unsigned int hash = 2166136261u;
        for (int i = 0; i < length; i++) {
            hash ^= static_cast<unsigned char>(str[i]);
            hash *= 16777619u;
        }
        return hash;
    }

    static void Initialize() {
        if (_initialized) return;
        _initialized = true;
        Intern("");
        Intern("True");
        Intern("False");
        Intern("null");
        Intern("\n");
        Intern(" ");
    }

public:
    static const char* Intern(const char* str) {
        if (!str) return nullptr;
        return Intern(str, static_cast<int>(std::strlen(str)));
    }

    static const char* Intern(const char* str, int length) {
        Initialize();
        if (!str) return nullptr;
        if (length < 0) length = 0;

        unsigned int hash = Hash(str, length);
        int bucket = static_cast<int>(hash % TABLE_SIZE);

        Entry* entry = _table[bucket];
        while (entry) {
            if (entry->hash == hash &&
                entry->length == length &&
                (length == 0 || std::memcmp(entry->str, str, length) == 0)) {
                return entry->str;
            }
            entry = entry->next;
        }

        Entry* newEntry = new Entry();
        newEntry->hash = hash;
        newEntry->length = length;
        newEntry->str = new char[length + 1];
        if (length > 0) {
            std::memcpy(newEntry->str, str, length);
        }
        newEntry->str[length] = '\0';
        newEntry->next = _table[bucket];
        _table[bucket] = newEntry;

        return newEntry->str;
    }

    static bool IsInterned(const char* str) {
        Initialize();
        if (!str) return false;
        int len = static_cast<int>(std::strlen(str));
        unsigned int hash = Hash(str, len);
        int bucket = static_cast<int>(hash % TABLE_SIZE);

        Entry* entry = _table[bucket];
        while (entry) {
            if (entry->str == str) return true;
            if (entry->hash == hash &&
                entry->length == len &&
                std::memcmp(entry->str, str, len) == 0) {
                return true;
            }
            entry = entry->next;
        }
        return false;
    }

    static int Count() {
        Initialize();
        int count = 0;
        for (int i = 0; i < TABLE_SIZE; i++) {
            Entry* entry = _table[i];
            while (entry) {
                count++;
                entry = entry->next;
            }
        }
        return count;
    }

    static const char* True()  { Initialize(); return Intern("True"); }
    static const char* False() { Initialize(); return Intern("False"); }
    static const char* Empty() { Initialize(); return Intern(""); }
};

// Static member initialization
StringIntern::Entry* StringIntern::_table[StringIntern::TABLE_SIZE] = { nullptr };
bool StringIntern::_initialized = false;

// ============================================================================
// Tests
// ============================================================================

void test_memory_pool() {
    std::cout << "\n=== MemoryPool Tests ===\n";

    TEST("Create pool");
    try {
        MemoryPool pool(32, 10);
        if (pool.BlockCount() == 10 && pool.FreeCount() == 10) {
            PASS();
        } else {
            FAIL("Wrong counts");
        }
    } catch (...) {
        FAIL("Exception thrown");
    }

    TEST("Allocate single block");
    {
        MemoryPool pool(32, 10);
        void* ptr = pool.Allocate();
        if (ptr != nullptr && pool.FreeCount() == 9 && pool.UsedCount() == 1) {
            PASS();
        } else {
            FAIL("Allocation failed");
        }
    }

    TEST("Free block");
    {
        MemoryPool pool(32, 10);
        void* ptr = pool.Allocate();
        pool.Free(ptr);
        if (pool.FreeCount() == 10 && pool.UsedCount() == 0) {
            PASS();
        } else {
            FAIL("Free failed");
        }
    }

    TEST("Allocate all blocks");
    {
        MemoryPool pool(32, 5);
        void* ptrs[5];
        for (int i = 0; i < 5; i++) {
            ptrs[i] = pool.Allocate();
        }
        bool allValid = true;
        for (int i = 0; i < 5; i++) {
            if (!ptrs[i]) allValid = false;
        }
        if (allValid && pool.FreeCount() == 0 && pool.IsEmpty()) {
            PASS();
        } else {
            FAIL("Not all blocks allocated");
        }
    }

    TEST("Allocate from exhausted pool returns nullptr");
    {
        MemoryPool pool(32, 2);
        pool.Allocate();
        pool.Allocate();
        void* ptr = pool.Allocate();
        if (ptr == nullptr) {
            PASS();
        } else {
            FAIL("Should return nullptr");
        }
    }

    TEST("Reset pool");
    {
        MemoryPool pool(32, 5);
        pool.Allocate();
        pool.Allocate();
        pool.Reset();
        if (pool.FreeCount() == 5 && pool.IsFull()) {
            PASS();
        } else {
            FAIL("Reset failed");
        }
    }

    TEST("Move constructor");
    {
        MemoryPool pool1(32, 10);
        pool1.Allocate();
        pool1.Allocate();
        MemoryPool pool2(std::move(pool1));
        if (pool2.BlockCount() == 10 && pool2.UsedCount() == 2) {
            PASS();
        } else {
            FAIL("Move failed");
        }
    }

    TEST("Reuse freed blocks");
    {
        MemoryPool pool(32, 3);
        void* p1 = pool.Allocate();
        void* p2 = pool.Allocate();
        pool.Free(p1);
        void* p3 = pool.Allocate();
        // p3 should reuse p1's block
        if (p3 == p1 && pool.UsedCount() == 2) {
            PASS();
        } else {
            FAIL("Block not reused");
        }
    }
}

void test_string_intern() {
    std::cout << "\n=== StringIntern Tests ===\n";

    TEST("Intern same string returns same pointer");
    {
        const char* s1 = StringIntern::Intern("Hello");
        const char* s2 = StringIntern::Intern("Hello");
        if (s1 == s2) {
            PASS();
        } else {
            FAIL("Pointers differ");
        }
    }

    TEST("Intern different strings returns different pointers");
    {
        const char* s1 = StringIntern::Intern("Hello");
        const char* s2 = StringIntern::Intern("World");
        if (s1 != s2) {
            PASS();
        } else {
            FAIL("Pointers same");
        }
    }

    TEST("Interned string has correct content");
    {
        const char* s = StringIntern::Intern("TestString");
        if (strcmp(s, "TestString") == 0) {
            PASS();
        } else {
            FAIL("Content mismatch");
        }
    }

    TEST("Empty string interning");
    {
        const char* s1 = StringIntern::Intern("");
        const char* s2 = StringIntern::Empty();
        if (s1 == s2 && strlen(s1) == 0) {
            PASS();
        } else {
            FAIL("Empty string issue");
        }
    }

    TEST("Pre-interned True/False");
    {
        const char* t = StringIntern::True();
        const char* f = StringIntern::False();
        if (strcmp(t, "True") == 0 && strcmp(f, "False") == 0 && t != f) {
            PASS();
        } else {
            FAIL("True/False issue");
        }
    }

    TEST("IsInterned returns true for interned string");
    {
        const char* s = StringIntern::Intern("UniqueTest123");
        if (StringIntern::IsInterned(s)) {
            PASS();
        } else {
            FAIL("Should be interned");
        }
    }

    TEST("Count increases with new strings");
    {
        int before = StringIntern::Count();
        StringIntern::Intern("NewUniqueString456");
        int after = StringIntern::Count();
        if (after > before) {
            PASS();
        } else {
            FAIL("Count didn't increase");
        }
    }

    TEST("Intern with explicit length");
    {
        const char* s = StringIntern::Intern("HelloWorld", 5);
        if (strcmp(s, "Hello") == 0) {
            PASS();
        } else {
            FAIL("Length not respected");
        }
    }

    TEST("Same substring interned correctly");
    {
        const char* s1 = StringIntern::Intern("Hello");
        const char* s2 = StringIntern::Intern("HelloWorld", 5);
        if (s1 == s2) {
            PASS();
        } else {
            FAIL("Substring should match");
        }
    }

    TEST("Null returns nullptr");
    {
        const char* s = StringIntern::Intern(nullptr);
        if (s == nullptr) {
            PASS();
        } else {
            FAIL("Should return nullptr");
        }
    }
}

int main() {
    std::cout << "====================================\n";
    std::cout << "Memory Classes Native Test\n";
    std::cout << "====================================\n";

    test_memory_pool();
    test_string_intern();

    std::cout << "\n====================================\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed\n";
    std::cout << "====================================\n";

    return failed > 0 ? 1 : 0;
}
