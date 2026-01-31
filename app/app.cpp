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
        std::cout << "Memory initialized at " << memory.get_m_start() << "\n";

        size_t s;
        void* ptr;
        while(1){
            std::cout << "Enter allocation size" << "\n";
            std::cin >> s;
            if(s == 0) break;

            ptr = memory.allocate(s);
            if(!ptr) break;

            std::cout << "Allocated at " << ptr << "\n";
        }
    }

    std::cout << "Memory block deleted" << "\n";

    return 0;
}