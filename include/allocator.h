#pragma once
#include <cstddef>

struct Header
{
    u_int32_t size;
    u_int32_t padding;
};


class Allocator{
    public:
        explicit Allocator(size_t size);
        ~Allocator();

        Allocator(const Allocator&) = delete;               // No copy constructor
        Allocator &operator=(const Allocator&) = delete;    // No copy assignment

        [[nodiscard]] void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));

        size_t get_total_capacity() const { return size; };
        size_t get_used_capacity() const { return static_cast<size_t>(m_cursor - m_start); };
        size_t get_remaining_capacity() const { return static_cast<size_t>(m_end - m_cursor); };
        void* get_m_start() const {return m_start;};

    private:
        std::byte* m_start;
        std::byte* m_cursor;
        std::byte* m_end;
        size_t size;
        size_t page_count;
};