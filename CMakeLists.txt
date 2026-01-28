# Define cmake version required
cmake_minimum_required(VERSION 3.20)
# Define project name and language (cpp)
project(allocator LANGUAGES CXX)

# Disable compiler-specific cpp extensions for portability
set(CMAKE_CXX_EXTENSIONS OFF)

# Define the library 'allocator' (implementation)
# Define the executable 'alloc_test' (cli tool)
add_library(allocator)
add_executable(app)

# Link library to source files
target_sources(allocator
    PRIVATE
        src/allocator.cpp
)

# Link executable to source files
target_sources(app
    PRIVATE
        app/app.cpp
)

# Allocator library will link to headers
target_include_directories(allocator
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Enforce cpp20
target_compile_features(allocator
    PUBLIC
        cxx_std_20
)

# MSVC: Microsoft Visual C++ (different flags than mac/linux)
# -Wall and -Wextra: Enable broad set of warnings for stricter code structure
# -Wpedantic: Prevents usage of compiler specific features
# -Werror: Treat warnings as errors
if(MSVC)
    set(WARNING_FLAGS /W4 /WX)
else()
    set(WARNING_FLAGS -Wall -Wextra -Wpedantic -Werror)
endif()

# Apply to Library
target_compile_options(allocator PRIVATE ${WARNING_FLAGS})

# Apply to Executable
target_compile_options(app PRIVATE ${WARNING_FLAGS})

# Add sanitizers to warn of silent failures
# If in debug mode and not on microsoft visual c++
# Add "sensor" code to warn of silent failures (example: integer overflow, out of bounds mem access)
if(CMAKE_BUILD_TYPE MATCHES Debug AND NOT MSVC)
    target_compile_options(allocator PRIVATE -fsanitize=address,undefined)
    target_link_options(allocator PRIVATE -fsanitize=address,undefined)

    target_compile_options(app PRIVATE -fsanitize=address,undefined)
    target_link_options(app PRIVATE -fsanitize=address,undefined)
endif()

# Link app to allocator
target_link_libraries(app
    PRIVATE
        allocator
)
