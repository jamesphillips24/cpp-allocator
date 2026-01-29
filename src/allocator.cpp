#include "allocator.h"
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

long PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

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

void* Allocator::allocate(size_t size, size_t allignment){
    size_t padding = (allignment - (reinterpret_cast<u_int64_t>(m_cursor) % allignment)) % allignment;

    m_cursor += padding;
    void* ptr = m_cursor;
    m_cursor += size;

    return ptr;
}