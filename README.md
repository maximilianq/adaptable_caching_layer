
# Adaptable Caching Layer (ACL)

The **Adaptable Caching Layer (ACL)** is a flexible caching layer designed to provide efficient caching and prefetching capabilities. ACL can be dynamically preloaded using `LD_PRELOAD` and customized using various environment variables. The library supports the implementation of custom caching policies and prefetching strategies, and also includes a custom benchmark tool for performance evaluation.

## Installation

To build the library, use the following commands:

```
mkdir build
cd build
cmake ..
make
```

Ensure that the `build/external` folder is added to your `LD_LIBRARY_PATH`:

```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/external
```

## Usage

### Setting Environment Variables

Before running your application with ACL, set the following environment variables:

-   **`ACL_CACHE_PATH`**: Directory where the cache will be stored.
-   **`ACL_SOURCE_PATH`**: Directory to be cached.
-   **`ACL_CAPACITY`**: Maximum cache size (in bytes).
-   **`ACL_STRATEGY`**: Prefetching strategy (`ABFP`, `FSDP` or `MCFP`).
-   **`ACL_POLICY`**: Caching policy (`LRU`, `LFU` or `FIFO`).
    
### Configuration Example
```
export ACL_CACHE_PATH=/tmp/acl_cache
export ACL_SOURCE_PATH=/data/source_folder
export ACL_CAPACITY=209715200  # 200 MB
export ACL_STRATEGY=sequential
export ACL_POLICY=LFU
```

### Run the Application with LD_PRELOAD
```
LD_PRELOAD=/path/to/libacl.so ./my_application
```

## Application Interface

### Informed Prefetching
Applications can disclose upcoming file access patterns using the `acl_advise` function:

```
void acl_advise(const char *path, int flag);
```

- `path`: path of the file that will soon be accessed.
- `flag`: advice flag (`ACL_WILLNEED` or `ACL_WONTNEED`).

### Custom Prefetching Strategies
To implement a custom prefetching strategy, define the following functions and pass the prefetch_strategy_t struct to the `acl_prefetch` function. The files to be moved into cache must be selected by passing the path to the file to the `acl_select` function.

```
typedef void (* init_prefetch_t)(prefetch_t * memory);
typedef void (* free_prefetch_t)(prefetch_t * memory);
typedef void (* process_prefetch_t)(prefetch_t * memory, char * path);

typedef struct prefetch_strategy {
    init_prefetch_t ps_init;
    free_prefetch_t ps_free;
    process_prefetch_t ps_process;
} prefetch_strategy_t;

void acl_prefetch(prefetch_strategy_t strategy);

void acl_select(char * path);
```

### Custom Caching Policies
To implement a custom prefetching strategy, define the following functions and pass the cache_policy_t struct to the `acl_cache` function:

```
typedef void (* init_cache_t)(cache_t * cache);
typedef void (* free_cache_t)(cache_t * cache);
typedef void (* insert_cache_t)(cache_t * cache, cache_entry_t * entry);
typedef void (* inserted_cache_t)(cache_t * cache, cache_entry_t * entry);
typedef cache_entry_t * (* remove_cache_t)(cache_t * cache, char * path);
typedef void (* removed_cache_t)(cache_t * cache, cache_entry_t * entry);
typedef cache_entry_t * (* retrieve_cache_t)(cache_t * cache, char * path);
typedef cache_entry_t * (* pop_cache_t)(cache_t * cache);

typedef struct cache_policy {
    init_cache_t cp_init;
    free_cache_t cp_free;
    insert_cache_t cp_insert;
    remove_cache_t cp_remove;
    retrieve_cache_t cp_retrieve;
    pop_cache_t cp_pop;
} cache_policy_t;

void acl_cache(cache_policy_t policy);
```

### Required Header
To access these functions simply include the following header in your custom implementations:

```
#include <external/acl.h>
```

# Custom Benchmark
The library includes a custom benchmarking tool that can be executed with the following command:

```
./custom_benchmark --path=data/custom/file_%d.data --pattern=read --files=32 --size=67108864 --seed=1234567890 --window=10 --blocksize=65536 --runtime=15 --interval=1024 --iteration=1024 --init --will
```

#### Benchmark Options
- `--path`: Path for the files used in the benchmark.
- `--pattern`: Access pattern (`read`, `randread`, `write`, `randwrite`).
- `--files`: Number of files to benchmark.
- `--size`: Size of each file (in bytes).
- `--seed`: RNG seed for consistent benchmarking results.
- `--window`: Number of files disclosed in advance.
- `--blocksize`: Size of each I/O operation (in bytes).
- `--runtime`: Total runtime of the benchmark (in seconds).
- `--interval`: Length of the repeating access pattern.
- `--iteration`: Number of operations per file before moving to the next.
- `--init`: Creates the required files in the specified directory before benchmarking.
- `--will`: Advises the future access of a file through the application interface.
- `--wont`: Advises the future absence of access of a file through the application interface.
- 
#### Example Configurations
Multiple example configurations are available in the folder benchmarks of the repository.