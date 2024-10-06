#define CACHE_SIZE ((int) (1048576.0 * 25.0 * 0.9))
//#define CACHE_SIZE ((int) (131072.0 * 625 * 0.8))

#define CACHE_POLICY_LRU
#define FETCH_STRATEGY_MCFL

#define SOURCE_PATH "/media/quaeck/storage"
//#define SOURCE_PATH "/home/quaeck/CLionProjects/ld-preload-benchmark/storage"
//#define SOURCE_PATH "/mnt/nfs4"

#define CACHE_PATH "/var/cache/iocache"