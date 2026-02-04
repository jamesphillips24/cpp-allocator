#include "allocator.h"
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>


int main(int argc, char* argv[]){
    if (argc != 2)
    {
        std::cout << "Run command: ./app {size of mem}" << "\n";
        exit(1);
    }
    size_t size = static_cast<size_t>(std::stoi(argv[1]));

    std::cout << getpid() << "\n";

    {
        Allocator memory(size);
        void** ptrs = new void*[10];
        int i = 0;
        std::cout << "Memory initialized at " << memory.get_m_start() << "\n";

        size_t s;
        while(1){
            std::cout << "Enter allocation size" << "\n";
            std::cin >> s;
            if(s == 0) break;

            ptrs[i] = memory.allocate(s);
            std::cout<<"M_cursor:" << memory.get_used_capacity() + static_cast<std::byte*>(memory.get_m_start())<<"\n";
            i++;
            std::cout<<"Here"<<"\n";
            if(!(ptrs[i-1])) break;
            memory.free(ptrs[i-1]);
            i--;
        }
    }

    std::cout << "Memory block deleted" << "\n";

    return 0;
}