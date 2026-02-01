#include "allocator.h"
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

const u_int32_t PAGE_SIZE = sysconf(_SC_PAGE_SIZE);
const u_int32_t HEADER_SIZE = sizeof(Header);
const u_int32_t FOOTER_SIZE = sizeof(Footer);

Allocator::Allocator(size_t size)
    {
        // Enforce page alignment
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

// Padding size is assumed to fit within 1 byte (safe for alignments less than 256)
void* Allocator::allocate(size_t size, size_t alignment){
    // TODO: Add size = max(size, free struct) to avoid corruption

    size_t current_cursor = reinterpret_cast<uintptr_t>(m_cursor);

    // Get offset to next block alignment considering header size and padding byte
    size_t padding = (alignment - ((current_cursor + HEADER_SIZE) % alignment)) % alignment;
    if(padding == 0) padding += alignment; // Make room for padding byte

    size_t total_block = HEADER_SIZE + padding + size + FOOTER_SIZE;
    if(m_cursor + total_block > m_end) return nullptr;

    // Header is up against previous footer so it can be located
    // when previous block is freed (to check for free block merging)
    Header *h = (Header *)(m_cursor);
    h->size = total_block | 1; // Set last bit to 1 to indicate allocated
    h->padding = padding;

    m_cursor += padding + HEADER_SIZE;
    void* ptr = m_cursor;
    ((char*)ptr)[-1] = padding; /* Write padding size to byte before ptr
    so header can be found*/
    m_cursor += size + FOOTER_SIZE;

    Footer *f = (Footer *)(m_cursor - FOOTER_SIZE);
    f->size = total_block | 1; // Set last bit to 1 to indicate allocated

    return ptr;
}