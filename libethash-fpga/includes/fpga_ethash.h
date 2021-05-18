#include "kec.h"


#define WORD_BYTES 4
#define DATASET_BYTES_INIT 1073741824U
#define DATASET_BYTES_GROWTH  8388608U
#define CACHE_BYTES_INIT 16777216U
#define CACHE_BYTES_GROWTH 131072U
#define CACHE_MULTIPLIER 1024
#define EPOCH_LENGTH 30000U

#define MIX_BYTES 128
#define HASH_BYTES 64
#define DATASET_PARENTS 256
#define CACHE_ROUNDS 3
#define ACCESSES 64

#define NODE_WORDS (64/4)
#define MIX_WORDS (MIX_BYTES/4)
#define MIX_NODES (MIX_WORDS / NODE_WORDS)

#define FNV_PRIME 0x01000193

// h256_t - 32 Bytes
typedef union
{
    uint8_t b[32];
} h256_t;

// node - 64 Bytes
typedef union
{
    uint8_t bytes[NODE_WORDS * 4];
    uint32_t words[NODE_WORDS];
    uint64_t double_words[NODE_WORDS / 2];
} node;

struct light
{
    void* cache;
    uint64_t cache_size;
    uint64_t block_number;
};

struct eth_full
{
    FILE* file;
    uint64_t file_size;
    node* data;
};

typedef struct eth_return_value
{
    h256_t result;
    h256_t mix_hash;
    bool success;
} ret_val_t;

typedef struct light* light_t;
typedef struct eth_full* full_t;
typedef int(*callback_t)(unsigned);

