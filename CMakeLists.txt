cmake_minimum_required(VERSION 3.22)
project(ld_preload_benchmark C)

set(CMAKE_C_STANDARD 17)

add_library(acl SHARED
        library/operations.c
        library/queue.c
        examples/bytes_read.c
        library/path.c
        library/file.c
        library/calls.c
        library/lookahead.c
        library/cleanup.c
        library/intercept.c
        library/cache.c
)

target_link_libraries(acl dl)
target_link_libraries(acl pthread)

add_executable(bytes_read examples/bytes_read.c)