James' Project

Configure make files (from build/): cmake ..

Run with (from build/): cmake --build . && ./app {memory block size}

Allocates a large block of memory with mmap in which the user
can allocate their own data structures. Automatically writes
a header structure of 8 bytes holding the size of the user memory
block and any padding added before hand (to prevent fragmentation
after free).