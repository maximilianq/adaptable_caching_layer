cmake_minimum_required(VERSION 3.22)
project(ld_preload_benchmark C)

set(CMAKE_C_STANDARD 17)

add_library(acl SHARED
        library/operations.c
        library/structures/queue.c
        examples/bytes_read.c
        library/path.c
        library/file.c
        library/calls.c
        library/intercept.c
        library/policies/lru.c
        parallel.c
        library/policies/lfu.c
        min_heap.c
        library/structures/heap.c
        test.c
        library/strategies/fsdl.c
        library/strategies/frdl.c
        library/directory.h
        library/directory.c
        library/strategies/psdl.c
        library/strategies/prdl.c
        examples/bytes_write.c
)

target_link_libraries(acl dl)
target_link_libraries(acl pthread)

add_executable(bytes_read examples/bytes_read.c)