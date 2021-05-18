#include<cstdlib>
#include<cstring>
#include<stdint.h>
#include<cmath>

// #include "libethash-fpga/ethash_fpga.h"

#define DATA_SIZE 4096

#define WORD_BYTES = 4
#define DATASET_BYTES_INIT = 1073741824U
#define DATASET_BYTES_GROWTH = 8388608U
#define CACHE_BYTES_INIT = 1073741824U
#define CACHE_BYTES_GROWTH = 131072U
#define CACHE_MULTIPLIER = 1024
#define EPOCH_LENGTH = 30000U

#define MIX_BYTES 128
#define HASH_BYTES 64
#define DATASET_PARENTS 256
#define CACHE_ROUNDS 3
#define ACCESSES 64

#define DAG_MAGIC_NUM_SIZE 8
#define DAG_MAGIC_NUM 0xFEE1DEADBADDCAFE





#define FNV_PRIME 0x01000193
static inline unsigned int fnv_hash(const unsigned int x, const unsigned int y)
{
	return x*FNV_PRIME ^ y;
}



static inline void SHA3_256(unsigned char * const ret, unsigned char const *data, const size_t size) {
	sha3_256(ret, 32, data, size);
}

static inline void SHA3_512(unsigned char * const ret, unsigned char const *data, const size_t size) {
	sha3_512(ret, 64, data, size);
}




// compile time settings
#define NODE_WORDS (64/4)
#define MIX_WORDS (MIX_BYTES/4)
#define MIX_NODES (MIX_WORDS / NODE_WORDS)



/*** Constants. ***/
const unsigned char rho[24] = 
		{ 1,  3,   6, 10, 15, 21,
	28, 36, 45, 55,  2, 14,
	27, 41, 56,  8, 25, 43,
	62, 18, 39, 61, 20, 44};
const unsigned char pi[24] = 
		{10,  7, 11, 17, 18, 3,
	5, 16,  8, 21, 24, 4,
	15, 23, 19, 13, 12, 2,
	20, 14, 22,  9, 6,  1};
static const uint64_t rc[24] = {
    0x0000000000000001, 0x0000000000008082, 0x800000000000808a, 0x8000000080008000,
    0x000000000000808b, 0x0000000080000001, 0x8000000080008081, 0x8000000000008009,
    0x000000000000008a, 0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
    0x000000008000808b, 0x800000000000008b, 0x8000000000008089, 0x8000000000008003,
    0x8000000000008002, 0x8000000000000080, 0x000000000000800a, 0x800000008000000a,
    0x8000000080008081, 0x8000000000008080, 0x0000000080000001, 0x8000000080008008};





static inline uint64_t rol(uint64_t x, unsigned s)
{
	return (((x) << s) | ((x) >> (64 - s)));
}

static inline void xorin(unsigned char* dst, const unsigned char* src, size_t len)
{
	for(size_t i=0; i < len; i++)
	{
		dst[i] ^= src[i];
	}
}
static inline void setout(const unsigned char* src, unsigned char* dst, size_t len)
{
	for(size_t i=0; i < len; i++)
	{
		dst[i] = src[i];
	}
}





#define P keccakf
#define Plen 200




// keccak-f[1600]
void keccakf(void* state)
{

	uint64_t a[25];
    memcpy(a, state, sizeof(a));

    uint64_t E[25];
    uint64_t B[5];
    uint64_t D[5];

    for (size_t n = 0; n < 24; n += 2)
    {

        for(unsigned int y=0; y<5; y++)
        {    
            B[y] = a[y] ^ a[y+5] ^ a[y+10] ^ a[y+15] ^ a[y+20];
        }

        for(unsigned int i=0; i<5; i++)
        {    
            D[i] = B[(i+1)%5] ^ rol(B[(i+4)%5], 1);
        }

        B[0] = a[0] ^ D[0];
        B[1] = rol(a[6] ^ D[1], 44);
        B[2] = rol(a[12] ^ D[2], 43);
        B[3] = rol(a[18] ^ D[3], 21);
        B[4] = rol(a[24] ^ D[4], 14);

        E[0] = B[0] ^ (~B[1] & B[2]) ^ rc[n];
        for(unsigned int i=1; i<5; i++)
        {    
            E[i] = B[i] ^ (~B[(i+1)%5] & B[(i+2)%5]);
        }

        B[0] = rol(a[3] ^ D[3], 28);
        B[1] = rol(a[9] ^ D[4], 20);
        B[2] = rol(a[10] ^ D[0], 3);
        B[3] = rol(a[16] ^ D[1], 45);
        B[4] = rol(a[22] ^ D[2], 61);

        for(unsigned int i=0; i<5; i++)
        {    
            E[i+5] = B[i] ^ (~B[(i+4)%5] & B[(i+3)%5]);
        }

        B[0] = rol(a[1] ^ D[1], 1);
        B[1] = rol(a[7] ^ D[2], 6);
        B[2] = rol(a[13] ^ D[3], 25);
        B[3] = rol(a[19] ^ D[4], 8);
        B[4] = rol(a[20] ^ D[5], 18);

        for(unsigned int i=0; i<5; i++)
        {    
            E[i+10] = B[i] ^ (~B[(i+4)%5] & B[(i+3)%5]);
        }

        B[0] = rol(a[4] ^ D[4], 27);
        B[1] = rol(a[6] ^ D[0], 36);
        B[2] = rol(a[11] ^ D[1], 10);
        B[3] = rol(a[17] ^ D[2], 15);
        B[4] = rol(a[23] ^ D[3], 56);

        for(unsigned int i=0; i<5; i++)
        {    
            E[i+15] = B[i] ^ (~B[(i+4)%5] & B[(i+3)%5]);
        }

        B[0] = rol(a[2] ^ D[2], 62);
        B[1] = rol(a[8] ^ D[3], 55);
        B[2] = rol(a[14] ^ D[4], 39);
        B[3] = rol(a[15] ^ D[0], 41);
        B[4] = rol(a[21] ^ D[1], 2);

        for(unsigned int i=0; i<5; i++)
        {    
            E[i+20] = B[i] ^ (~B[(i+4)%5] & B[(i+3)%5]);
        }


        // Round (n + 1): Exx -> Axx
        for(unsigned int y=0; y<5; y++)
        {    
            B[y] = E[y] ^ E[y+5] ^ E[y+10] ^ E[y+15] ^ E[y+20];
        }

        for(unsigned int i=0; i<5; i++)
        {    
            D[i] = B[(i+1)%5] ^ rol(B[(i+4)%5], 1);
        }

        B[0] = E[0] ^ D[0];
        B[1] = rol(E[6] ^ D[1], 44);
        B[2] = rol(E[12] ^ D[2], 43);
        B[3] = rol(E[18] ^ D[3], 21);
        B[4] = rol(E[24] ^ D[4], 14);
        
        a[0] = B[0] ^ (~B[1] & B[2]) ^ rc[n + 1];
        for(unsigned int i=1; i<5; i++)
        {    
            a[i] = B[i] ^ (~B[(i+4)%5] & B[(i+3)%5]);
        }

        B[0] = rol(E[3] ^ D[3], 28);
        B[1] = rol(E[9] ^ D[4], 20);
        B[2] = rol(E[10] ^ D[0], 3);
        B[3] = rol(E[16] ^ D[1], 45);
        B[4] = rol(E[22] ^ D[2], 61);

        for(unsigned int i=0; i<5; i++)
        {    
            a[i+5] = B[i] ^ (~B[(i+4)%5] & B[(i+3)%5]);
        }

        B[0] = rol(E[1] ^ D[1], 1);
        B[1] = rol(E[7] ^ D[2], 6);
        B[2] = rol(E[13] ^ D[3], 25);
        B[3] = rol(E[19] ^ D[4], 8);
        B[4] = rol(E[20] ^ D[5], 18);

        for(unsigned int i=0; i<5; i++)
        {    
            a[i+10] = B[i] ^ (~B[(i+4)%5] & B[(i+3)%5]);
        }

        B[0] = rol(E[4] ^ D[4], 27);
        B[1] = rol(E[6] ^ D[0], 36);
        B[2] = rol(E[11] ^ D[1], 10);
        B[3] = rol(E[17] ^ D[2], 15);
        B[4] = rol(E[23] ^ D[3], 56);

        for(unsigned int i=0; i<5; i++)
        {    
            a[i+15] = B[i] ^ (~B[(i+4)%5] & B[(i+3)%5]);
        }

        B[0] = rol(E[2] ^ D[2], 62);
        B[1] = rol(E[8] ^ D[3], 55);
        B[2] = rol(E[14] ^ D[4], 39);
        B[3] = rol(E[15] ^ D[0], 41);
        B[4] = rol(E[21] ^ D[1], 2);

        for(unsigned int i=0; i<5; i++)
        {    
            a[i+20] = B[i] ^ (~B[(i+4)%5] & B[(i+3)%5]);
        }

    }

    memcpy(state, a, sizeof(a));

}



int hash(unsigned char* out, size_t outlen, const unsigned char* in, size_t inlen, size_t rate, unsigned char delim)
{
	unsigned char a[Plen];

	//absorb input.
	while (inlen >= rate)
	{
		xorin(a, in, rate);
		P(a);
		in += rate;
		inlen -= rate;
	}

	// Xor in the DS and pad frame
	a[inlen] ^= delim;
	a[rate - 1] = 0x80;

	//Xor in the last block
	xorin(a, in, inlen);


	// Apply KeccakF
	P(a);


	// Squeeze output
	while (outlen >= rate)
	{
		setout(a, out, rate);
		P(a);
		out += rate;
		outlen -= rate;
	}
	setout(a, out, outlen);

	for(unsigned int i=0; i<Plen; i++)
	{
		a[i] = 0;
	}

	return 0;
}




int sha3_256(unsigned char* out, size_t outlen, const unsigned char* in, size_t inlen)
{
	if(outlen > 256/8)
	{
		return -1;
	}
	return hash(out, outlen, in, inlen, 200 - (256/4), 0x01);
}

int sha3_512(unsigned char* out, size_t outlen, const unsigned char* in, size_t inlen)
{
	if(outlen > 512/8)
	{
		return -1;
	}
	return hash(out, outlen, in, inlen, 200 - (512/4), 0x01);
}


#define DAG_SIZE 1073739904U

typedef union
{
	unsigned char bytes[32 / sizeof(unsigned char)];
	unsigned int words[32 / sizeof(unsigned int)];
	unsigned long double_words[32 / sizeof(unsigned long)];
} hash32_t;

typedef union
{
	unsigned char bytes[NODE_WORDS * 4];
	unsigned int words[NODE_WORDS];
	unsigned long double_words[NODE_WORDS / 2];
} node64_t;

void search(
		hash32_t* ret_mix,
		hash32_t* ret_hash, //s+mix
		node64_t* full_nodes, //dag
		const hash32_t* header_hash,
		const unsigned int nonce)
{

	node64_t s_mix[MIX_NODES + 1];
	hash32_t hash;

	memcpy(s_mix[0].bytes, header_hash, 32);
	s_mix[0].double_words[4] = nonce;

	// compute sha3-512 hash and replicate across mix
	SHA3_512(s_mix->bytes, s_mix->bytes, 40);

	node64_t* mix = s_mix + 1;
	for (unsigned w = 0; w != MIX_WORDS; ++w) {
		mix->words[w] = s_mix[0].words[w % NODE_WORDS];
	}


	unsigned const full_size = (unsigned) DAG_SIZE;
	unsigned const num_full_pages = (unsigned) (full_size / MIX_BYTES);


	unsigned int index;
	node64_t dag_node;
	for (unsigned i = 0; i != ACCESSES; ++i) {
		index = ((s_mix->words[0] ^ i) * FNV_PRIME ^ mix->words[i % MIX_WORDS]) % num_full_pages;

		for (unsigned n = 0; n != MIX_NODES; ++n) {
			dag_node = full_nodes[MIX_NODES * index + n];

			for (unsigned w = 0; w != NODE_WORDS; ++w) {
				mix[n].words[w] = fnv_hash(mix[n].words[w], dag_node.words[w]);
			}
		}
	}

	// compress mix (length reduced from 128 to 32 bytes)
	for (unsigned w = 0; w != MIX_WORDS; w += 4) {
		uint reduction = mix->words[w + 0];
		reduction = reduction * FNV_PRIME ^ mix->words[w + 1];
		reduction = reduction * FNV_PRIME ^ mix->words[w + 2];
		reduction = reduction * FNV_PRIME ^ mix->words[w + 3];
		mix->words[w / 4] = reduction;
	}

	//memcpy(ret_mix, mix->bytes, 32);
	for (unsigned i = 0; i < 32/4; i++) {
		ret_mix->words[i] = mix->words[i];
	}
	// final Keccak hash
	SHA3_256(hash.bytes, s_mix->bytes, 64 + 32); // Keccak-256(s + compressed_mix)
	// copy from local mem to global
	for (unsigned i = 0; i < 32/4; i++) {
		ret_hash->words[i] = hash.words[i];
	}

}



bool isprime(uint64_t x)
{
	for(uint64_t i=2; i<(int)sqrt(x); i++)
	{
		if(x % i == 0)
			return false;
	}
	return true;
}


uint64_t get_cache_size(uint64_t block_number)
{
	uint64_t sz = (uint64_t)pow(2,30) + (uint64_t)pow(2,17) * (block_number / 30000);

	sz -= HASH_BYTES;

	while(! isprime(sz / HASH_BYTES) )
	{
		sz -= 2 * HASH_BYTES;
	}
	return sz;
}

uint64_t get_full_dag_size(uint64_t block_number)
{
	uint64_t sz = (uint64_t)pow(2,30) + (uint64_t)pow(2,17) * (block_number / 30000);
	sz -= MIX_BYTES;
	while(! isprime(sz / MIX_BYTES) )
	{
		sz -= 2 * MIX_BYTES;
	}
	return sz;
}

uint64_t* mkcache(uint64_t cache_size, uint32_t seed)
{
	uint64_t n = cache_size / HASH_BYTES;

	uint64_t* o = new 
}



void GenerateDag(
	uint64_t dag_size,
	uint32_t gridSize,
	uint32_t blockSize)
	// unsigned int start,
	// const unsigned int* _Cache, 
	// unsigned int* _DAG0,
	// unsigned int* _DAG1,
	// unsigned int light_size)
{

	// const node64_t* Cache = (node64_t*) _Cache;
	// const unsigned int gid = 0;
	// unsigned int NodeIdx = start + gid;
	// const unsigned int thread_id = gid & 3;

	const uint32_t work = 



}




