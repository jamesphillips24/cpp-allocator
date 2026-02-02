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
        f_head = nullptr;
    }

Allocator::~Allocator(){
    if(m_start != nullptr){
        if(munmap(m_start, size) == -1){
            std::cerr << "ERROR: Unmap failed" << "\n";
        }
    }
}

// Padding size is assumed to fit within 1 byte (safe for alignments less than 256)
// TODO: Separate functionality in to functions and then fill in pseudocode
void* Allocator::allocate(size_t size, size_t alignment){
    size = std::max(size, sizeof(FreeBlock)); // Otherwise the free block overwrites headers

    /* Psuedocode for free list traversal
    if(f_head != nullptr){
        FreeBlock* f_tmp = f_head;
        unsigned char *f_tmp_char;

        while(f_tmp != nullptr){
            f_tmp_char = reinterpret_cast<unsigned char*>(f_tmp);
            Header *h_tmp = reinterpret_cast<Header *>(f_tmp_char - f_tmp_char[-1] - HEADER_SIZE);

            size_t padding_tmp = get_padding(h_tmp, alignment);
            if(h_tmp->size >= padding_tmp + HEADER_SIZE + size + FOOTER_SIZE){
                void* ptr = reinterpret_cast<unsigned char*>(h_tmp) + HEADER_SIZE + padding_tmp;
                // Allocate block here (including writing header, padding byte, and footer)
                // Change header and footer to 'allocated' status
                if(f_tmp->prev != nullptr){
                    f_tmp->prev->next = f_tmp->next;
                }
                else{
                    f_head = f_tmp->next;
                }
                if(f_tmp->next != nullptr){
                        f_tmp->next->prev = f_tmp->prev;
                }

                return ptr;
            }

            f_tmp = f_tmp->next;
        }
    }
    */

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

    if(!f_head){
        f_head = (FreeBlock*)m_cursor;
        f_head->prev = nullptr;
        f_head->next = nullptr;
    }

    return ptr;
}