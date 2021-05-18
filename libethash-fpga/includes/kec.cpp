#include "kec.h"

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

    const uint8_t *buf = (uint8_t*)bufIn;

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
