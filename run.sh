mkdir build;
gcc -fPIC -ldl -shared -pthread -o build/libacl.so library/operations.c library/queue.c library/path.c library/calls.c library/file.c library/lookahead.c library/cleanup.c library/intercept.c;
gcc -o build/bytes_read.o examples/bytes_read.c;