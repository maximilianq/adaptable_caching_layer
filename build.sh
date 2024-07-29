mkdir build;
gcc -fPIC -ldl -shared -pthread -o build/libacl.so library/operations.c library/structures/queue.c library/structures/heap.c library/path.c library/calls.c library/file.c library/intercept.c library/directory.c library/policies/lru.c library/policies/lfu.c library/strategies/fsdl.c library/strategies/psdl.c library/strategies/frdl.c library/strategies/prdl.c;
gcc -o build/bytes_read.o examples/bytes_read.c;
gcc -o build/bytes_write.o examples/bytes_write.c;