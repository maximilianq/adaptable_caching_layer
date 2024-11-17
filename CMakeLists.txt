cmake_minimum_required(VERSION 3.10)

# Project Name
project(MyProject)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/library/include)

# Set the output directories for external library
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/external)

set(SOURCE_PATH "/home/quaeck/whythehell/adaptable_caching_layer/build/data")
set(CACHE_PATH "/home/quaeck/whythehell/adaptable_caching_layer/build/cache")

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
        ${CMAKE_SOURCE_DIR}/library/cache.c
        ${CMAKE_SOURCE_DIR}/library/calls.c
        ${CMAKE_SOURCE_DIR}/library/intercept.c
        ${CMAKE_SOURCE_DIR}/library/utils/directory.c
        ${CMAKE_SOURCE_DIR}/library/utils/file.c
        ${CMAKE_SOURCE_DIR}/library/utils/path.c
        ${CMAKE_SOURCE_DIR}/library/utils/string.c
        ${CMAKE_SOURCE_DIR}/library/structures/internal/markov.c
        ${CMAKE_SOURCE_DIR}/library/structures/markov.c
        ${CMAKE_SOURCE_DIR}/library/structures/internal/extent.c
        ${CMAKE_SOURCE_DIR}/library/structures/extent.c
        ${CMAKE_SOURCE_DIR}/library/structures/internal/priority.c
        ${CMAKE_SOURCE_DIR}/library/structures/priority.c
        ${CMAKE_SOURCE_DIR}/library/structures/internal/lookup.c
        ${CMAKE_SOURCE_DIR}/library/structures/lookup.c
        ${CMAKE_SOURCE_DIR}/library/structures/internal/queue.c
        ${CMAKE_SOURCE_DIR}/library/structures/queue.c
        ${CMAKE_SOURCE_DIR}/library/structures/internal/heap.c
        ${CMAKE_SOURCE_DIR}/library/structures/heap.c
        ${CMAKE_SOURCE_DIR}/library/prefetch.c
        ${CMAKE_SOURCE_DIR}/library/prefetch.c
        ${CMAKE_SOURCE_DIR}/library/mapping.c
        ${CMAKE_SOURCE_DIR}/library/prefetch/fsdp.c
        ${CMAKE_SOURCE_DIR}/library/prefetch/mcfp.c
        ${CMAKE_SOURCE_DIR}/library/prefetch/abfp.c
        ${CMAKE_SOURCE_DIR}/library/cache/fifo.c
        ${CMAKE_SOURCE_DIR}/library/cache/lru.c
        ${CMAKE_SOURCE_DIR}/library/cache/lfu.c
)

target_compile_definitions(acl PRIVATE SOURCE_PATH="${SOURCE_PATH}" CACHE_PATH="${CACHE_PATH}")

target_link_libraries(acl dl pthread)
target_link_options(acl PRIVATE "-Wl,-init,acl_init")
set_target_properties(acl PROPERTIES OUTPUT_NAME "acl")

add_executable(prefetch_seq_read ${CMAKE_SOURCE_DIR}/examples/prefetch_seq_read.c)
target_compile_definitions(prefetch_seq_read PRIVATE SOURCE_PATH="${SOURCE_PATH}")
target_link_libraries(prefetch_seq_read ${CMAKE_BINARY_DIR}/external/libacl.so)

add_executable(prefetch_rand_read ${CMAKE_SOURCE_DIR}/examples/prefetch_rand_read.c)
target_compile_definitions(prefetch_rand_read PRIVATE SOURCE_PATH="${SOURCE_PATH}")
target_link_libraries(prefetch_rand_read ${CMAKE_BINARY_DIR}/external/libacl.so)

add_executable(prefetch_seq_write ${CMAKE_SOURCE_DIR}/examples/prefetch_seq_write.c)
target_compile_definitions(prefetch_seq_write PRIVATE SOURCE_PATH="${SOURCE_PATH}")
target_link_libraries(prefetch_seq_write ${CMAKE_BINARY_DIR}/external/libacl.so)

add_executable(prefetch_rand_write ${CMAKE_SOURCE_DIR}/examples/prefetch_rand_write.c)
target_compile_definitions(prefetch_rand_write PRIVATE SOURCE_PATH="${SOURCE_PATH}")
target_link_libraries(prefetch_rand_write ${CMAKE_BINARY_DIR}/external/libacl.so)