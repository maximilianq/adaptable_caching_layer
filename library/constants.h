#define CACHE_SIZE ((int) (1048576.0 * 25.0 * 1.0))
//#define CACHE_SIZE ((int) (131072.0 * 25.0 * 1.0))

#define CACHE_POLICY_LRU
#define FETCH_STRATEGY_PSDL

//#define SOURCE_PATH "/media/quaeck/storage"
//#define SOURCE_PATH "/home/quaeck/CLionProjects/ld-preload-benchmark/storage"
#define SOURCE_PATH "/mnt/nfs4"

#define CACHE_PATH "/var/cache/iocache"