cmake_minimum_required(VERSION 3.10)

# Project Name
project(AdaptableCachingLayer)

# Set the output directories for external library
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/external)

# Create the external library libacl.so
add_library(acl_external SHARED ${CMAKE_SOURCE_DIR}/source/external/acl.c)
target_link_libraries(acl_external dl)
set_target_properties(acl_external PROPERTIES OUTPUT_NAME "acl")

# Set the output directories for libraries and executables
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Create the main library libacl.so
add_library(acl SHARED
        ${CMAKE_SOURCE_DIR}/source/acl.c
        ${CMAKE_SOURCE_DIR}/source/cache.c
        ${CMAKE_SOURCE_DIR}/source/calls.c
        ${CMAKE_SOURCE_DIR}/source/intercept.c
        ${CMAKE_SOURCE_DIR}/source/utils/directory.c
        ${CMAKE_SOURCE_DIR}/source/utils/file.c
        ${CMAKE_SOURCE_DIR}/source/utils/path.c
        ${CMAKE_SOURCE_DIR}/source/utils/string.c
        ${CMAKE_SOURCE_DIR}/source/structures/internal/markov.c
        ${CMAKE_SOURCE_DIR}/source/structures/markov.c
        ${CMAKE_SOURCE_DIR}/source/structures/internal/extent.c
        ${CMAKE_SOURCE_DIR}/source/structures/extent.c
        ${CMAKE_SOURCE_DIR}/source/structures/internal/priority.c
        ${CMAKE_SOURCE_DIR}/source/structures/priority.c
        ${CMAKE_SOURCE_DIR}/source/structures/internal/lookup.c
        ${CMAKE_SOURCE_DIR}/source/structures/lookup.c
        ${CMAKE_SOURCE_DIR}/source/structures/internal/queue.c
        ${CMAKE_SOURCE_DIR}/source/structures/queue.c
        ${CMAKE_SOURCE_DIR}/source/structures/internal/heap.c
        ${CMAKE_SOURCE_DIR}/source/structures/heap.c
        ${CMAKE_SOURCE_DIR}/source/prefetch.c
        ${CMAKE_SOURCE_DIR}/source/prefetch.c
        ${CMAKE_SOURCE_DIR}/source/mapping.c
        ${CMAKE_SOURCE_DIR}/source/prefetch/fsdp.c
        ${CMAKE_SOURCE_DIR}/source/prefetch/mcfp.c
        ${CMAKE_SOURCE_DIR}/source/prefetch/abfp.c
        ${CMAKE_SOURCE_DIR}/source/cache/fifo.c
        ${CMAKE_SOURCE_DIR}/source/cache/lru.c
        ${CMAKE_SOURCE_DIR}/source/cache/lfu.c
)

target_compile_definitions(acl PRIVATE SOURCE_PATH="${SOURCE_PATH}" CACHE_PATH="${CACHE_PATH}")

target_link_libraries(acl dl pthread)
target_link_options(acl PRIVATE "-Wl,-init,acl_init")
set_target_properties(acl PROPERTIES OUTPUT_NAME "acl")

add_executable(custom_benchmark ${CMAKE_SOURCE_DIR}/source/benchmark/custom_benchmark.c)
target_link_libraries(custom_benchmark ${CMAKE_BINARY_DIR}/external/libacl.so)