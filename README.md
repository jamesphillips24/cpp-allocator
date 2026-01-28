James' Project

Configure make files (from build/): cmake ..

Run with (from build/): cmake --build . && ./app {memory block size}

How to test allocator (Unix):
- Run program
- Take pid and run vmmap {pid}
- Find VM_ALLOCATE (reserved) and see the reserved memory
- Press enter to continue through destructor
- Run vmmap again and see reserved memory is gone
