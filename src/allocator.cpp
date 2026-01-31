#include "allocator.h"
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

const long PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

Allocator::Allocator(size_t size)
    {
        page_count = size / PAGE_SIZE;
        if(size%PAGE_SIZE) page_count++;
        this->size = page_count * PAGE_SIZE;

        void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        if(ptr == MAP_FAILED){
            throw std::bad_alloc();
        }

        m_start = static_cast<std::byte*>(ptr);
        m_cursor = m_start;
        m_end = m_start + this->size;
    }

Allocator::~Allocator(){
    if(m_start != nullptr){
        if(munmap(m_start, size) == -1){
            std::cerr << "ERROR: Unmap failed" << "\n";
        }
    }
}

// Ptr aligned in memory, header sizeof(Header) bytes before alignment
void* Allocator::allocate(size_t size, size_t alignment){
    size_t current_cursor = reinterpret_cast<uintptr_t>(m_cursor);
    size_t padding = (alignment - ((current_cursor + sizeof(Header)) % alignment)) % alignment;

    size_t adjustment = padding + sizeof(Header);
    size_t total_required = adjustment + size;
    if(m_cursor + total_required > m_end) return nullptr;

    m_cursor += adjustment;

    Header* h = (Header*)(m_cursor - sizeof(Header));
    h->size = size;
    h->padding = padding;

    void* ptr = m_cursor;
    m_cursor += size;

    return ptr;
}