#include<stdint.h>
#include<stdio.h>
#include<cstdlib>
#include<cmath>
#include<ethash/ethash.hpp>

#define DATA_SIZE 4096

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

#define DAG_MAGIC_NUM_SIZE 8
#define DAG_MAGIC_NUM 0xFEE1DEADBADDCAFE


// compile time settings
#define NODE_WORDS (64/4)
#define MIX_WORDS (MIX_BYTES/4)
#define MIX_NODES (MIX_WORDS / NODE_WORDS)


#define FNV_PRIME 0x01000193


#if defined(_MSC_VER)
#define SHA3_CONST(x) x
#else
#define SHA3_CONST(x) x##L
#endif

static const uint64_t keccakf_rndc[24] = {
    SHA3_CONST(0x0000000000000001UL), SHA3_CONST(0x0000000000008082UL),
    SHA3_CONST(0x800000000000808aUL), SHA3_CONST(0x8000000080008000UL),
    SHA3_CONST(0x000000000000808bUL), SHA3_CONST(0x0000000080000001UL),
    SHA3_CONST(0x8000000080008081UL), SHA3_CONST(0x8000000000008009UL),
    SHA3_CONST(0x000000000000008aUL), SHA3_CONST(0x0000000000000088UL),
    SHA3_CONST(0x0000000080008009UL), SHA3_CONST(0x000000008000000aUL),
    SHA3_CONST(0x000000008000808bUL), SHA3_CONST(0x800000000000008bUL),
    SHA3_CONST(0x8000000000008089UL), SHA3_CONST(0x8000000000008003UL),
    SHA3_CONST(0x8000000000008002UL), SHA3_CONST(0x8000000000000080UL),
    SHA3_CONST(0x000000000000800aUL), SHA3_CONST(0x800000008000000aUL),
    SHA3_CONST(0x8000000080008081UL), SHA3_CONST(0x8000000000008080UL),
    SHA3_CONST(0x0000000080000001UL), SHA3_CONST(0x8000000080008008UL)
};

static const unsigned keccakf_rotc[24] = {
    1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14, 27, 41, 56, 8, 25, 43, 62,
    18, 39, 61, 20, 44
};

static const unsigned keccakf_piln[24] = {
    10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4, 15, 23, 19, 13, 12, 2, 20,
    14, 22, 9, 6, 1
};


#ifndef ROTL64
#define ROTL64(x, y) \
	(((x) << (y)) | ((x) >> ((sizeof(uint64_t)*8) - (y))))
#endif


#define KECCAK_SPONGE_WORDS \
		(((1600)/8)/sizeof(uint64_t))

#define SHA3_USE_KECCAK_FLAG 0x80000000
#define SHA3_CW(x) ((x) & (~SHA3_USE_KECCAK_FLAG))

typedef struct sha3_context_
{
	uint64_t saved;

	union
	{
		uint64_t s[KECCAK_SPONGE_WORDS];
		uint8_t sb[KECCAK_SPONGE_WORDS * 8];
	} u;

	unsigned byteIndex;
	unsigned wordIndex;
	unsigned capacityWords;
	
} sha3_context;



void kec_init(void* priv, unsigned bitSize);
void kec_256_init(void* priv);
void kec_512_init(void* priv);
void kec_update(void* priv, void const* bufIn, size_t len);
void const* kec_finalize(void* priv);
void kec_HashBuffer(unsigned bitSize, void const* in, size_t inBytes, void* out, unsigned outBytes);



typedef union
{
    uint8_t uints8[256];
    uint32_t uints32[64];
    uint64_t uints64[32];
} hash32_bytes_t;

typedef union
{   
    uint8_t uints8[512];
    uint32_t uints32[128];
    uint64_t uints64[64];
} hash64_bytes_t;

// node_words = 16
typedef union  
{
	uint8_t bytes[16 * 4];
	uint32_t words[16];
	uint64_t double_words[16 / 2];
} hash_node;


// 32 bytes
typedef union
{
    uint8_t b[32];
} h256_t;

struct light {
	void* cache;
	uint64_t cache_size;
	uint64_t block_number;
};

struct ethash_full {
	FILE* file;
	uint64_t file_size;
	hash_node* data;
};

struct light;
typedef struct light* light_t;
struct full;
typedef struct full* full_t;
typedef int(*callback_t)(unsigned);


typedef struct ethash_return_value 
{
    hash_256_t result;
    hash_256_t mix_hash;
    bool success;
} return_value_t;
