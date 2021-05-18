#include "data_sizes.h"
#include "fpga_ethash.h"
#include <cmath>

static inline uint32_t fnv_hash(uint32_t const x, uint32_t const y)
{
	return x*FNV_PRIME ^ y;
}

// uint64_t E_epoch(uint64_t block_number)
// {
//     return (uint64_t)(block_number / EPOCH_LENGTH);
// }
// uint64_t E_prime(uint64_t x, uint64_t y)
// {
//     if(x%y == 0)
//     {
//         return x;
//     }
//     else
//     {
//         return E_prime(x-2*y,y);
//     }
// }

uint64_t get_d_size(uint64_t const block_number)
{
    // calculation
    // return E_prime(DATASET_BYTES_INIT+DATASET_BYTES_GROWTH*E_epoch(block_number)-MIX_BYTES, MIX_BYTES);
    // assert(block_number / ETHASH_EPOCH_LENGTH < 2048);
    return dag_sizes[ (uint64_t) (block_number / EPOCH_LENGTH) ];
}

uint64_t get_c_size(uint64_t const block_number)
{
    // calculation
    // return E_prime(CACHE_BYTES_INIT+CACHE_BYTES_GROWTH*E_epoch(block_number)-HASH_BYTES, HASH_BYTES);
    // assert(block_number / ETHASH_EPOCH_LENGTH < 2048);
    return cache_sizes[ (uint64_t) (block_number / EPOCH_LENGTH) ];
}

h256_t get_seedhash(uint64_t const block_number)
{
    h256_t ret;
    memset(&ret, 0, 32);

    uint64_t const epochs = block_number / EPOCH_LENGTH;
    for (uint32_t i = 0; i < epochs; i++)
    {
        kec_HashBuffer(256, (uint8_t)&ret, 32, &ret, 32);
    }

    return ret;
}

bool static compute_cache_nodes(node* const nodes, uint64_t cache_size, h256_t const* seed)
{
    if(cache_size % sizeof(node) != 0)
    {
        return false;
    }
    uint32_t const num_nodes = (uint32_t) (cache_size / sizeof(node));

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
            node data;
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

void compute_dag_item(node* const ret, uint32_t node_index, light_t const light)
{

    uint32_t num_parent_nodes = (uint32_t) (light->cache_size / sizeof(node));
    node const* cache_nodes = (node const*) light->cache;
    node const* init = &cache_nodes[node_index % num_parent_nodes];
    memcpy(ret, init, sizeof(node));
    ret->words[0] ^= node_index;
    kec_HashBuffer(512, ret->bytes, sizeof(node), ret->bytes, 64);

    for (uint32_t i = 0; i != DATASET_PARENTS; ++i)
    {
        uint32_t parent_index = fnv_hash(node_index ^ i, ret->words[i % 16]) % num_parent_nodes;
        node const* parent = &cache_nodes[parent_index];

        for(unsigned w=0; w != 16; ++w)
        {
            ret->words[w] = fnv_hash(ret->words[w], parent->words[w]);
        }
    }

    kec_HashBuffer(512, ret->bytes, sizeof(node), ret->bytes, 64);
}

bool compute_full_dag(void* mem, uint64_t full_size, light_t const light, callback_t callback)
{
    if(full_size % (sizeof(uint32_t) * MIX_BYTES/4) != 0 ||
        (full_size % sizeof(node)) != 0)
    {
        return false;
    }

    uint32_t const max_n = (uint32_t)(full_size / sizeof(node));
    node* full_nodes = (node*) mem;
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


// ============================= KERNELS =================================

void krnl_ethash_func(
    h256_t* ret_res,
    h256_t* ret_mix,
    node const* full_nodes,
    light_t const light,
    h256_t const header_hash,
    uint64_t const nonce)
{
    
    node s_mix[MIX_NODES + 1];
    memcpy(s_mix[0].bytes, &header_hash, 32);
    // fix_endian64(s_mix);
    s_mix[0].double_words[4] = nonce;
    
    
    
    // CREATING MIX 0
    kec_HashBuffer(512, s_mix->bytes, 40, s_mix->bytes, 64);
    node* mix = s_mix + 1;
    for (unsigned w = 0; w != MIX_WORDS; ++w) {
		mix->words[w] = s_mix[0].words[w % NODE_WORDS];
	}


    uint64_t const full_size = get_d_size(light->block_number);
    uint64_t const num_full_pages = (uint64_t) (full_size / MIX_BYTES);



    // mixing and MEMORY BANDWIDTH PART 
    uint32_t index;
    node dag_node;

    for (unsigned i = 0; i < ACCESSES; ++i)
    {
        // HERE WE HAVE, WHERE TO TAKE FROM INFO FROM DAG
        index = ((s_mix->words[0] ^ i) * FNV_PRIME ^ mix->words[i % MIX_WORDS]) % num_full_pages;

        for (unsigned n = 0; n != MIX_NODES; ++n)
        {
            // TODO : look into just generating this item instead of bandwidth
            dag_node = full_nodes[MIX_NODES * index + n];

            for (unsigned w=0; w!=NODE_WORDS; ++w)
            {
                mix[n].words[w] = fnv_hash(mix[n].words[w], dag_node.words[w]);
            }
        }
    }
    

    // compress mix (len reduced from 128 to 32 bytes)
    for (unsigned w = 0; w < MIX_WORDS; w+=4)
    {
        uint32_t reduction = mix->words[w+0];
        reduction = reduction * FNV_PRIME ^ mix->words[w+1];
        reduction = reduction * FNV_PRIME ^ mix->words[w+2];
        reduction = reduction * FNV_PRIME ^ mix->words[w+3];
        mix->words[w/4] = reduction;
    }
    
    // memcpy(mix_hash, mix->bytes, 32);

}






void krnl_search(
    volatile struct SearchResutls* g_output,
    const h256_t g_header,
    node const* _g_dag0,
    uint64_t full_size,
    uint64_t start_nonce,
    uint64_t target)
{






}


void krnl_generateDAGitem(node* const ret, uint32_t node_index, light_t const light)
{
    uint32_t num_parent_nodes = (uint32_t) (light->cache_size / sizeof(node));
    node const* cache_nodes = (node const*) light->cache;
    node const* init = &cache_nodes[node_index % num_parent_nodes];
    memcpy(ret, init, sizeof(node));
    ret->words[0] ^= node_index;
    kec_HashBuffer(512, ret->bytes, sizeof(node), ret->bytes, 64);

    for (uint32_t i = 0; i != DATASET_PARENTS; ++i)
    {
        uint32_t parent_index = fnv_hash(node_index ^ i, ret->words[i % 16]) % num_parent_nodes;
        node const* parent = &cache_nodes[parent_index];

        for(unsigned w=0; w != 16; ++w)
        {
            ret->words[w] = fnv_hash(ret->words[w], parent->words[w]);
        }
    }

    kec_HashBuffer(512, ret->bytes, sizeof(node), ret->bytes, 64);
}

// mem holds the 
void krnl_generateDAG(uint32_t start, void* mem, uint64_t full_size, light_t const light, callback_t callback)
{

    if(full_size % (sizeof(uint32_t) * MIX_BYTES/4) != 0 ||
        (full_size % sizeof(node)) != 0)
    {
        return;// false;
    }


    uint32_t const max_n = (uint32_t)(full_size / sizeof(node));
    node* full_nodes = (node*) mem;
    double const progress_change = 1.0f / max_n;
    double progress = 0.0f;


    uint32_t num_parent_nodes = (uint32_t) (light->cache_size / sizeof(node));
    node const* cache_nodes = (node const*) light->cache;

    // compute full nodes
    for (uint32_t n = 0; n != max_n; ++n)
    {
        if( callback &&
            n % (max_n / 100) == 0 &&
            callback((unsigned int)(ceil(progress * 100.0f))) != 0)
        {
            return;
        }
        progress += progress_change;

        // compute_dag_item(&(full_nodes[n]), n, light);
        
        node const* init = &cache_nodes[n % num_parent_nodes];
        memcpy(&(full_nodes[n]), init, sizeof(node));
        
        full_nodes->words[0] ^= n;

        kec_HashBuffer(512, full_nodes->bytes, sizeof(node), full_nodes->bytes, 64);

        for (uint32_t i = 0; i != DATASET_PARENTS; ++i)
        {
            uint32_t parent_index = fnv_hash(n ^ i, full_nodes->words[i % 16]) % num_parent_nodes;
            node const* parent = &cache_nodes[parent_index];

            for(unsigned w=0; w != 16; ++w)
            {
                full_nodes->words[w] = fnv_hash(full_nodes->words[w], parent->words[w]);
            }
        }

        kec_HashBuffer(512, full_nodes->bytes, sizeof(node), full_nodes->bytes, 64);

    }

}