#include "allocator.h"
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

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
        std::cout << "Memory initialized. Press enter to end" << "\n";
        std::cin.get();
    }

    std::cout << "Memory object deleted. Press enter to end" << "\n";
    std::cin.get();

    return 0;
}