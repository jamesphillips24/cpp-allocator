#include "allocator.h"
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

const u_int32_t PAGE_SIZE = sysconf(_SC_PAGE_SIZE);
const u_int32_t HEADER_SIZE = sizeof(Header);
const u_int32_t FOOTER_SIZE = sizeof(Footer);

// Get offset to next block alignment considering header size and padding byte
size_t get_padding(const size_t cursor, const size_t alignment){
    size_t padding = (alignment - ((cursor + HEADER_SIZE) % alignment)) % alignment;
    if (padding == 0)
        padding += alignment; // Make room for padding byte

    return padding;
}

void* Allocator::write_block(std::byte*& cursor, const size_t size, const size_t total_block, const size_t padding){
    // Header is up against previous footer so it can be located
    // when previous block is freed (to check for free block merging)
    Header *h = (Header *)(cursor);
    h->size = total_block | 1; // Set last bit to 1 to indicate allocated
    h->padding = padding;

    cursor += padding + HEADER_SIZE;
    void *ptr = cursor;
    ((char *)ptr)[-1] = padding; /* Write padding size to byte before ptr
     so header can be found*/
    cursor += size + FOOTER_SIZE;

    Footer *f = (Footer *)(cursor - FOOTER_SIZE);
    f->size = total_block | 1; // Set last bit to 1 to indicate allocated

    return ptr;
}

Allocator::Allocator(size_t size)
    {
        // Enforce page alignment
        page_count = size / PAGE_SIZE;
        if(size%PAGE_SIZE) page_count++;
        this->size = page_count * PAGE_SIZE;

        void* ptr = mmap(NULL, this->size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
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
// TODO after making free function: Handle if there's one free block with whole
// arena empty
void* Allocator::allocate(size_t size, size_t alignment){
    size = std::max(size, sizeof(FreeBlock)); // Otherwise the free block overwrites headers

    if(f_head != nullptr){
        FreeBlock* f_tmp = f_head;
        unsigned char *f_tmp_char;
        size_t total_block;

        while(f_tmp != nullptr){
            f_tmp_char = reinterpret_cast<unsigned char*>(f_tmp);
            Header *h_tmp = reinterpret_cast<Header *>(f_tmp_char - f_tmp_char[-1] - HEADER_SIZE);
            std::byte *h_tmp_byte = reinterpret_cast<std::byte *>(h_tmp);

            size_t padding_tmp = get_padding(reinterpret_cast<size_t>(h_tmp), alignment);
            total_block = HEADER_SIZE + padding_tmp + size + FOOTER_SIZE;
            if(h_tmp->size >= total_block){
                void* ptr = write_block(h_tmp_byte, size, total_block, padding_tmp);
                std::cout << "Allocated in free block at " << ptr << "\n";

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

    size_t current_cursor = reinterpret_cast<uintptr_t>(m_cursor);
    size_t padding = get_padding(current_cursor, alignment);

    size_t total_block = HEADER_SIZE + padding + size + FOOTER_SIZE;
    if(m_cursor + total_block > m_end) return nullptr;

    void* ptr = Allocator::write_block(m_cursor, size, total_block, padding);
    std::cout << "Allocated at m_cursor " << ptr << "\n";

    return ptr;
}