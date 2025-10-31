cmake_minimum_required(VERSION 3.14)

project(kleene VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Collect all header files from include directory
file(GLOB_RECURSE HEADER_FILES "${PROJECT_SOURCE_DIR}/include/*.h")

# Collect all source files from src directory (excluding main.cpp)
file(GLOB SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/main.cpp")

# Create a library from all source files (shared between main and tests)
add_library(parser_lib
    ${SOURCE_FILES}
    ${HEADER_FILES}  # Add all headers to make them visible in IDE
)

target_include_directories(parser_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Main executable
add_executable(kleene
    src/main.cpp
)

target_link_libraries(kleene PRIVATE parser_lib)

# Enable testing
enable_testing()

# Test executable
add_executable(test_exec
    tests/test.cpp
)

target_link_libraries(test_exec PRIVATE parser_lib)

# Add test to CTest
add_test(NAME ParserTests COMMAND test_exec)

