#pragma once
// =============================================================================
// memory_pool.hpp — Custom Slab Memory Pool Allocator
// Complexity: Alloc O(1), Dealloc O(1)
// Design: Pre-allocated contiguous block with intrusive free-list
// =============================================================================
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <cassert>
#include <mutex>
#include <vector>

namespace billing::core {

// ---------------------------------------------------------------------------
// MemoryPool<T> — Fixed-size object pool
// ---------------------------------------------------------------------------
template <typename T, std::size_t BlockSize = 4096>
class MemoryPool {
public:
    static constexpr std::size_t OBJECT_SIZE = sizeof(T) > sizeof(void*) ? sizeof(T) : sizeof(void*);

    MemoryPool() : free_list_(nullptr), blocks_allocated_(0), total_objects_(0) {}

    ~MemoryPool() {
        for (auto* block : blocks_) {
            std::free(block);
        }
    }

    // Non-copyable, non-movable
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    // Allocate one object slot — O(1)
    T* allocate() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!free_list_) {
            allocate_block();
        }
        // Pop from free list
        void* slot = free_list_;
        free_list_ = *reinterpret_cast<void**>(slot);
        total_objects_++;
        return reinterpret_cast<T*>(slot);
    }

    // Return object to pool — O(1)
    void deallocate(T* ptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        // Push onto free list
        *reinterpret_cast<void**>(ptr) = free_list_;
        free_list_ = ptr;
        total_objects_--;
    }

    // Construct object in-place
    template <typename... Args>
    T* construct(Args&&... args) {
        T* slot = allocate();
        new (slot) T(std::forward<Args>(args)...);
        return slot;
    }

    // Destroy object and return to pool
    void destroy(T* ptr) {
        ptr->~T();
        deallocate(ptr);
    }

    std::size_t total_objects() const { return total_objects_; }
    std::size_t blocks_allocated() const { return blocks_allocated_; }

private:
    void allocate_block() {
        // Calculate how many objects fit per block
        constexpr std::size_t objects_per_block = BlockSize / OBJECT_SIZE;
        static_assert(objects_per_block > 0, "Object larger than block size");

        std::size_t block_bytes = objects_per_block * OBJECT_SIZE;
        void* block = std::malloc(block_bytes);
        if (!block) throw std::bad_alloc();

        blocks_.push_back(block);
        blocks_allocated_++;

        // Chain all slots into free list
        char* cursor = static_cast<char*>(block);
        for (std::size_t i = 0; i < objects_per_block - 1; ++i) {
            *reinterpret_cast<void**>(cursor) = cursor + OBJECT_SIZE;
            cursor += OBJECT_SIZE;
        }
        *reinterpret_cast<void**>(cursor) = free_list_;
        free_list_ = block;
    }

    void*                   free_list_;
    std::vector<void*>      blocks_;
    std::size_t             blocks_allocated_;
    std::size_t             total_objects_;
    std::mutex              mutex_;
};

// ---------------------------------------------------------------------------
// PoolAllocator<T> — STL-compatible allocator wrapping MemoryPool
// ---------------------------------------------------------------------------
template <typename T>
class PoolAllocator {
public:
    using value_type = T;

    explicit PoolAllocator(MemoryPool<T>& pool) : pool_(pool) {}

    T* allocate(std::size_t n) {
        if (n != 1) throw std::bad_alloc(); // pool only for single objects
        return pool_.allocate();
    }

    void deallocate(T* p, std::size_t) {
        pool_.deallocate(p);
    }

private:
    MemoryPool<T>& pool_;
};

} // namespace billing::core
