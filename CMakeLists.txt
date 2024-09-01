cmake_minimum_required(VERSION 3.10)

# Project Name
project(MyProject)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/library/include)

# Set the output directories for external library
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/external)

# Create the external library libacl.so
add_library(acl_external SHARED ${CMAKE_SOURCE_DIR}/library/external/acl.c)
target_link_libraries(acl_external dl)
set_target_properties(acl_external PROPERTIES OUTPUT_NAME "acl")

# Set the output directories for libraries and executables
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Create the main library libacl.so
add_library(acl SHARED
        ${CMAKE_SOURCE_DIR}/library/acl.c
        ${CMAKE_SOURCE_DIR}/library/memory.c
        ${CMAKE_SOURCE_DIR}/library/cache.c
        ${CMAKE_SOURCE_DIR}/library/calls.c
        ${CMAKE_SOURCE_DIR}/library/intercept.c
        ${CMAKE_SOURCE_DIR}/library/utils/directory.c
        ${CMAKE_SOURCE_DIR}/library/utils/file.c
        ${CMAKE_SOURCE_DIR}/library/utils/path.c
        ${CMAKE_SOURCE_DIR}/library/structures/internal/extent.c
        ${CMAKE_SOURCE_DIR}/library/structures/extent.c
        ${CMAKE_SOURCE_DIR}/library/structures/internal/priority.c
        ${CMAKE_SOURCE_DIR}/library/structures/priority.c
        ${CMAKE_SOURCE_DIR}/library/structures/internal/lookup.c
        ${CMAKE_SOURCE_DIR}/library/structures/lookup.c
        ${CMAKE_SOURCE_DIR}/library/structures/queue.c
        ${CMAKE_SOURCE_DIR}/library/structures/internal/heap.c
        ${CMAKE_SOURCE_DIR}/library/structures/heap.c
        ${CMAKE_SOURCE_DIR}/library/strategies/frdl.c
        ${CMAKE_SOURCE_DIR}/library/strategies/fsdl.c
        ${CMAKE_SOURCE_DIR}/library/strategies/prdl.c
        ${CMAKE_SOURCE_DIR}/library/strategies/psdl.c
)

target_link_libraries(acl dl pthread)
set_target_properties(acl PROPERTIES OUTPUT_NAME "acl")

# Create the executables
add_executable(bytes_read ${CMAKE_SOURCE_DIR}/examples/bytes_read.c)
target_link_libraries(bytes_read acl)

add_executable(bytes_write ${CMAKE_SOURCE_DIR}/examples/bytes_write.c)
target_link_libraries(bytes_write acl)