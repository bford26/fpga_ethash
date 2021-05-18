
#include "fpga_ethash.h"

// ================================

static void keccakf(uint64_t s[25])
{
    int i,j,round;
    uint64_t t, bc[5];

    for(round=0; round<24; round++)
    {
        // theta
        for(i=0; i<5; i++)
            bc[i] = s[i] ^ s[i+5] ^ s[i+10] ^ s[i+15] ^ s[i+20];

        for (i=0; i<5; i++)
        {
            t = bc[(i+4)%5] ^ ROTL64(bc[(i+1)%5],1);
            for (j=0; j<5; j++)
            {
                s[j+i] ^= t;
            }
        }

        // Rho Pi
        t = s[i];
        for (i=0; i<24; i++)
        {
            j = keccakf_piln[i];
            bc[0] = s[j];
            s[j] = ROTL64(t, keccakf_rotc[i]);
            t = bc[0];
        }

        // Chi
        for (j=0; j<25; j+=5)
        {
            for(i=0;i<5;i++)
                bc[i] = s[j+i];
            for(i=0;i<5;i++)
                s[j+i] ^= (~bc[(i+1)%5]) & bc[(i+2)%5];
        }
        
        // Iota
        s[0] ^= keccakf_rndc[round];
    }

}

void kec_init(void* priv, unsigned bitSize)
{
    sha3_context *ctx = (sha3_context*) priv;
    if(bitSize != 256 && bitSize != 512)
        printf("\nERROR : bitSize %d is not an option..\n", bitSize);

    memset(ctx, 0, sizeof(*ctx));
    ctx->capacityWords = 2*bitSize / (8*sizeof(uint64_t));
}
void kec_256_init(void* priv)
{
    kec_init(priv, 256);
}
void kec_512_init(void* priv)
{
    kec_init(priv, 512);
}

void kec_update(void* priv, void const* bufIn, size_t len)
{
    sha3_context *ctx = (sha3_context*) priv;

    unsigned old_tail = (8 - ctx->byteIndex) & 7;

    size_t words;
    unsigned tail;
    size_t i;

    const uint8_t *buf = bufIn;

    if(len < old_tail)
    {
        while( len--)
            ctx->saved |= (uint64_t) (*(buf++)) << ((ctx->byteIndex++) * 8);
        return;
    }

    if(old_tail)
    {
        len -= old_tail;
        while (old_tail)
        {
            ctx->saved |= (uint64_t) (*(buf++)) << ((ctx->byteIndex++) * 8);
        }
        
        ctx->u.s[ctx->wordIndex] ^= ctx->saved;

        ctx->byteIndex = 0;
        ctx->saved = 0;

        if(++ctx->wordIndex == (KECCAK_SPONGE_WORDS - SHA3_CW(ctx->capacityWords)))
        {
            keccakf(ctx->u.s);
            ctx->wordIndex = 0;
        }
    }


    words = len / sizeof(uint64_t);
    tail = len - words * sizeof(uint64_t);

    for(i=0; i<words; i++, buf+=sizeof(uint64_t))
    {
        const uint64_t t = (uint64_t) (buf[0]) |
                ((uint64_t) (buf[1]) << 8 * 1) |
                ((uint64_t) (buf[2]) << 8 * 2) |
                ((uint64_t) (buf[3]) << 8 * 3) |
                ((uint64_t) (buf[4]) << 8 * 4) |
                ((uint64_t) (buf[5]) << 8 * 5) |
                ((uint64_t) (buf[6]) << 8 * 6) |
                ((uint64_t) (buf[7]) << 8 * 7);

// #if defined(__x86_64__ ) || defined(__i386__)
//         SHA3_ASSERT(memcmp(&t, buf, 8) == 0);
// #endif

        ctx->u.s[ctx->wordIndex] ^= t;
        if(++ctx->wordIndex == (KECCAK_SPONGE_WORDS - SHA3_CW(ctx->capacityWords)))
        {
            keccakf(ctx->u.s);
            ctx->wordIndex = 0;
        }
    }


    // save the partial word
    while (tail--)
    {
        ctx->saved |= (uint64_t) (*(buf++)) << ((ctx->byteIndex++) * 8);
    }
}

void const* kec_finalize(void* priv)
{
    sha3_context *ctx = (sha3_context *) priv;

    uint64_t t;

    t = (uint64_t)(((uint64_t) 1) << (ctx->byteIndex * 8));

    ctx->u.s[ctx->wordIndex] ^= ctx->saved ^ t;

    ctx->u.s[KECCAK_SPONGE_WORDS - SHA3_CW(ctx->capacityWords) - 1] ^= SHA3_CONST(0x8000000000000000UL);

    keccakf(ctx->u.s);
    /* Return first bytes of the ctx->s. This conversion is not needed for
    * little-endian platforms e.g. wrap with #if !defined(__BYTE_ORDER__)
    * || !defined(__ORDER_LITTLE_ENDIAN__) || __BYTE_ORDER__!=__ORDER_LITTLE_ENDIAN__ 
    *    ... the conversion below ...
    * #endif */
   {
       unsigned i;
        for(i=0; i<KECCAK_SPONGE_WORDS; i++)
        {
            const unsigned t1 = (uint32_t) ctx->u.s[i];
            const unsigned t2 = (uint32_t) ( (ctx->u.s[i] >> 16) >> 16);
            ctx->u.sb[i * 8 + 0] = (uint8_t) (t1);
            ctx->u.sb[i * 8 + 1] = (uint8_t) (t1 >> 8);
            ctx->u.sb[i * 8 + 2] = (uint8_t) (t1 >> 16);
            ctx->u.sb[i * 8 + 3] = (uint8_t) (t1 >> 24);
            ctx->u.sb[i * 8 + 4] = (uint8_t) (t2);
            ctx->u.sb[i * 8 + 5] = (uint8_t) (t2 >> 8);
            ctx->u.sb[i * 8 + 6] = (uint8_t) (t2 >> 16);
            ctx->u.sb[i * 8 + 7] = (uint8_t) (t2 >> 24);
        }
   }

    return(ctx->u.sb);
}

void kec_HashBuffer(unsigned bitSize, void const* in, size_t inBytes, void* out, unsigned outBytes)
{
    sha3_context c;
    kec_init(&c, bitSize);

    kec_update(&c, in, inBytes);
    const void *h = kec_finalize(&c);

    if(outBytes > bitSize/8)
        outBytes = bitSize/8;

    memcpy(out, h,  outBytes);
}

// =================================================



static inline uint32_t fnv_hash(uint32_t const x, uint32_t const y)
{
	return x*FNV_PRIME ^ y;
}

static bool isprime(uint64_t x)
{
    for(uint64_t i=2; i<(uint64_t)sqrt(x); i++)
    {
        if(x%i == 0)
        {
            return false;
        }
    }
    return true;
}

uint64_t E_epoch(uint64_t block_number)
{
    return (uint64_t)(block_number / EPOCH_LENGTH);
}

uint64_t E_prime(uint64_t x, uint64_t y)
{
    if(x%y == 0)
    {
        return x;
    }
    else
    {
        return E_prime(x-2*y,y);
    }
}

uint64_t get_d_size(uint64_t block_number)
{
    return E_prime(DATASET_BYTES_INIT+DATASET_BYTES_GROWTH*E_epoch(block_number)-MIX_BYTES, MIX_BYTES);
}

uint64_t get_c_size(uint64_t block_number)
{
    return E_prime(CACHE_BYTES_INIT+CACHE_BYTES_GROWTH*E_epoch(block_number)-HASH_BYTES, HASH_BYTES);
}

uint64_t* get_seedhash(uint64_t block_number)
{
    // C_seedhash is 32 bytes
    uint64_t* C_seedhash = new uint64_t[ 32 / 8 ];

    if(E_epoch(block_number) != 0)
    {
        // KEC(get_seedhash(block_number - EPOCH_LENGTH))        
        kec_HashBuffer(256, get_seedhash(block_number - EPOCH_LENGTH), 32, C_seedhash, 32);
    }
    else
    {
        for(unsigned int i=0; i<32/8; i++)
        {
            C_seedhash[i] = 0;
        }
    }

    return C_seedhash;
}

bool static compute_cache_nodes(hash_node* const nodes, uint64_t cache_size, hash_256_t const* seed)
{
    if(cache_size % sizeof(hash_node) != 0)
    {
        return false;
    }
    uint32_t const num_nodes = (uint32_t) (cache_size / sizeof(hash_node));

    kec_HashBuffer(512, (uint8_t*)seed, 32, nodes[0].bytes, 64);

    for (uint32_t i = 1; i != num_nodes; ++i)
    {
        kec_HashBuffer(512, nodes[i-1].bytes, 64, nodes[i].bytes, 64);
    }
    
    for(uint32_t j=0; j != CACHE_ROUNDS; j++)
    {
        for (uint32_t i = 0; i != num_nodes; i++)
        {
            uint32_t const idx = nodes[i].words[0] % num_nodes;
            hash_node data;
            data = nodes[(num_nodes - 1 + i) % num_nodes];
            for(uint32_t w=0; w != 16; ++w)
            {
                data.words[w] ^= nodes[idx].words[w];
            }
            kec_HashBuffer(512, data.bytes, sizeof(data), nodes[i].bytes, 64);
        }
    }
    // fix endian!
    // fix_endian_arr32(nodes->words, num_nodes * 16);
    return true;
}

void compute_dag_item(hash_node* const ret, uint32_t node_index, light_t const light)
{

    uint32_t num_parent_nodes = (uint32_t) (light->cache_size / sizeof(hash_node));
    hash_node const* cache_nodes = (hash_node const*) light->cache;
    hash_node const* init = &cache_nodes[node_index % num_parent_nodes];
    memcpy(ret, init, sizeof(hash_node));
    ret->words[0] ^= node_index;
    kec_HashBuffer(512, ret->bytes, sizeof(hash_node), ret->bytes, 64);

    for (uint32_t i = 0; i != DATASET_PARENTS; ++i)
    {
        uint32_t parent_index = fnv_hash(node_index ^ i, ret->words[i % 16]) % num_parent_nodes;
        hash_node const* parent = &cache_nodes[parent_index];

        for(unsigned w=0; w != 16; ++w)
        {
            ret->words[w] = fnv_hash(ret->words[w], parent->words[w]);
        }
    }

    kec_HashBuffer(512, ret->bytes, sizeof(hash_node), ret->bytes, 64);
}

bool compute_full_dag(void* mem, uint64_t full_size, light_t const light, callback_t callback)
{
    if(full_size % (sizeof(uint32_t) * MIX_BYTES/4) != 0 ||
        (full_size % sizeof(hash_node)) != 0)
    {
        return false;
    }

    uint32_t const max_n = (uint32_t)(full_size / sizeof(hash_node));
    hash_node* full_nodes = (hash_node*) mem;
    double const progress_change = 1.0f / max_n;
    double progress = 0.0f;
    // compute full nodes
    for (uint32_t n = 0; n != max_n; ++n)
    {
        if( callback &&
            n % (max_n / 100) == 0 &&
            callback((unsigned int)(ceil(progress * 100.0f))) != 0)
        {
            return false;
        }
        progress += progress_change;
        compute_dag_item(&(full_nodes[n]), n, light);
    }
    return true;
}


// static uint64_t get_cache_size(uint64_t block_number)
// {
//     uint64_t sz = CACHE_BYTES_INIT + CACHE_BYTES_GROWTH + (block_number / EPOCH_LENGTH);
//     sz -= HASH_BYTES;
//     while (!isprime(sz / HASH_BYTES))
//     {
//         sz-=2*HASH_BYTES;
//     }
//     return sz;
// }
// static uint64_t get_dataset_size(uint64_t block_number)
// {
//     uint64_t sz = DATASET_BYTES_INIT + DATASET_BYTES_GROWTH + (block_number / EPOCH_LENGTH);
//     sz -= MIX_BYTES;
//     while (!isprime(sz / MIX_BYTES))
//     {
//         sz-=2*MIX_BYTES;
//     }
//     return sz;
// }
// uint64_t* mkcache(uint64_t cache_size, uint64_t seed)
// {
//     uint64_t n = cache_size / HASH_BYTES;
//     // o = [sha3_512(seed)]
//     uint64_t* o;
//     o = new uint64_t[524288];
//     //
//     for (size_t i = 1; i < n; i++)
//     {
//         /* code */
//         // o.append(sha3_512(o[-1]))
//     }   
//     for (size_t i = 0; i < CACHE_ROUNDS; i++)
//     {
//         for (size_t j = 0; j < n; j++)
//         {
//             /* code */
//             //v = o[j][0] % n;
//             //o[j] = sha3_512(map(xor, o[(j-1+n) % n], o[v]))
//         }
//     }
//     return o;
// }
// void calc_dataset_item(uint64_t* cache, uint64_t i)
// {
//     uint64_t n = sizeof(cache)/sizeof(uint64_t);
//     uint64_t r = HASH_BYTES / WORD_BYTES;
//     // INIT THE MIX
//     uint64_t* mix;
//     //copy.copy(cache[i % n])
//     //mix[0] ^= i
//     //mix = sha3_512(mix)
//     // fnv it with a lot of random cache nodes based on i
//     for (uint64_t j = 0; j < DATASET_PARENTS; j++)
//     {
//         // uint64_t cache_index = fnv_hash(i ^ j, mix[j % r]);
//         // mix = map(fnv, mix, cache[cache_index % n]);
//     }
//     // return sha3_512(mix)
// }
// void calc_dataset(uint64_t full_size, uint64_t* cache)
// {
//     uint64_t* dag;
//     for (uint64_t i = 0; i < full_size / HASH_BYTES; i++)
//     {
//         calc_dataset_item(cache, i);
//     }
// }



void krnl_ethash_func(
    return_value_t* ret,
    hash_node const* full_nodes,
    light_t const light,
    uint64_t full_size,
    hash_256_t const header_hash,
    uint64_t const nonce)
{
    
    if (full_size % MIX_WORDS != 0) {
		return; //false;
	}

    // pack hash and nonce together into first 40 bytes of s_mix
	// assert(sizeof(hash_node) * 8 == 512);
	hash_node s_mix[MIX_NODES + 1];
	memcpy(s_mix[0].bytes, &header_hash, 32);
	// fix_endian64(s_mix[0].double_words[4], nonce);

    // compute sha3-512 hash and replicate across mix
	// done with function below ... SHA3_512(s_mix->bytes, s_mix->bytes, 40)
    kec_HashBuffer(512, s_mix->bytes, 40, s_mix->bytes, 64);

	// fix_endian_arr32(s_mix[0].words, 16);

    hash_node* const mix = s_mix + 1;
	for (uint32_t w = 0; w != MIX_WORDS; ++w) {
		mix->words[w] = s_mix[0].words[w % NODE_WORDS];
	}

    unsigned const page_size = sizeof(uint32_t) * MIX_WORDS;
	unsigned const num_full_pages = (unsigned) (full_size / page_size);

    for (unsigned i = 0; i != ACCESSES; ++i)
    {
		uint32_t const index = fnv_hash(s_mix->words[0] ^ i, mix->words[i % MIX_WORDS]) % num_full_pages;

		for (unsigned n = 0; n != MIX_NODES; ++n)
        {
			hash_node const* dag_node;
		
        	if (full_nodes) {
				dag_node = &full_nodes[MIX_NODES * index + n];
			} else {
				hash_node tmp_node;
				compute_dag_item(&tmp_node, index * MIX_NODES + n, light);
				dag_node = &tmp_node;
			}

            for (unsigned w = 0; w != NODE_WORDS; ++w) 
            {
                mix[n].words[w] = fnv_hash(mix[n].words[w], dag_node->words[w]);
            }	
		}
	}

    // compress mix
    for (uint32_t w = 0; w != MIX_WORDS; w += 4) {
		uint32_t reduction = mix->words[w + 0];
		reduction = reduction * FNV_PRIME ^ mix->words[w + 1];
		reduction = reduction * FNV_PRIME ^ mix->words[w + 2];
		reduction = reduction * FNV_PRIME ^ mix->words[w + 3];
		mix->words[w / 4] = reduction;
	}

    // fix_endian_arr32(mix->words, MIX_WORDS / 4);
	memcpy(&ret->mix_hash, mix->bytes, 32);
	// final Keccak hash
	// SHA3_256(&ret->result, s_mix->bytes, 64 + 32); // Keccak-256(s + compressed_mix)
    kec_HashBuffer(256, s_mix->bytes, 64+32, &ret->result, 32);

	// return true;

}
