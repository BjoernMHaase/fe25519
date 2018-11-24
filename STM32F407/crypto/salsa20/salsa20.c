/*                          =======================
  ============================ C/C++ HEADER FILE =============================
                            =======================

    \file salsa20.c

    Originally taken from avr-nacl project (Author: Michael Hutter, Peter Schwabe)

    \Author: B. Haase, Endress + Hauser Conducta GmbH & Co. KG

    License: CC0 1.0 (http://creativecommons.org/publicdomain/zero/1.0/legalcode)
  ============================================================================*/
#include "../include/crypto_stream.h"
#include "../include/crypto_target_config.h"
#include "../local_includes/bigint.h"
#include "../local_includes/salsa20defs.h"

#define ROUNDS 20

static uint32_t load_littleendian(const uint8_t * x)
{
    return (uint32_t)(x[0]) | (((uint32_t)(x[1])) << 8) | (((uint32_t)(x[2])) << 16) | (((uint32_t)(x[3])) << 24);
}

static void store_littleendian(uint8_t * x, uint32_t u)
{
    x[0] = (uint8_t)u;
    u >>= 8;
    x[1] = (uint8_t)u;
    u >>= 8;
    x[2] = (uint8_t)u;
    u >>= 8;
    x[3] = (uint8_t)u;
}

#ifdef CRYPTO_HAS_ASM_HSALSA20_BLOCK

#else

#define SALSA_QUARTERROUND(a, b, c, R)                                                                                                     \
    {                                                                                                                                      \
        uint32_t d = b + c;                                                                                                                \
        uint32_t e = d << R;                                                                                                               \
        d          = d >> (32 - R);                                                                                                        \
        d ^= e;                                                                                                                            \
        a ^= d;                                                                                                                            \
    }

#define SALSA20_LOAD(io, x, A, B, C, D)                                                                                                    \
    {                                                                                                                                      \
        io.a = x[A];                                                                                                                       \
        io.b = x[B];                                                                                                                       \
        io.c = x[C];                                                                                                                       \
        io.d = x[D];                                                                                                                       \
    }
#define SALSA20_STORE(io, x, A, B, C, D)                                                                                                   \
    {                                                                                                                                      \
        x[A] = io.a;                                                                                                                       \
        x[B] = io.b;                                                                                                                       \
        x[C] = io.c;                                                                                                                       \
        x[D] = io.d;                                                                                                                       \
    }
#define SALSA20_R(io, x, A, B, C, D)                                                                                                       \
    SALSA20_LOAD(io, x, A, B, C, D);                                                                                                       \
    io = crypto_salsa20_round(io);                                                                                                         \
    SALSA20_STORE(io, x, A, B, C, D);

typedef struct STSalsa20RoundInOut_
{
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
} STSalsa20RoundInOut;

static struct STSalsa20RoundInOut_ crypto_salsa20_round(struct STSalsa20RoundInOut_ io)
{
    SALSA_QUARTERROUND(io.a, io.b, io.c, 7);
    SALSA_QUARTERROUND(io.d, io.a, io.b, 9);
    SALSA_QUARTERROUND(io.c, io.d, io.a, 13);
    SALSA_QUARTERROUND(io.b, io.c, io.d, 18);

    return io;
}

void crypto_core_hsalsa20_block(volatile uint32_t * x)
{
    int ctr;

    for (ctr = ROUNDS; ctr > 0; ctr -= 2)
    {
        STSalsa20RoundInOut io;

        SALSA20_R(io, x, 4, 0, 12, 8);
        SALSA20_R(io, x, 9, 5, 1, 13);
        SALSA20_R(io, x, 14, 10, 6, 2);
        SALSA20_R(io, x, 3, 15, 11, 7);

        SALSA20_R(io, x, 1, 0, 3, 2);
        SALSA20_R(io, x, 6, 5, 4, 7);
        SALSA20_R(io, x, 11, 10, 9, 8);
        SALSA20_R(io, x, 12, 15, 14, 13);
    }
}
#endif

void crypto_core_salsa20_transform(uint32_t * inOut)
{
    uint32_t x[16];
    int ctr;

    for (ctr = 0; ctr < 16; ctr++)
    {
        x[ctr] = inOut[ctr];
    }

    crypto_core_hsalsa20_block(x);

    for (ctr = 0; ctr < 16; ctr++)
    {
        inOut[ctr] += x[ctr];
    }
}


int crypto_core_salsa20_aligned(uint32_t * out, const uint32_t * in, const uint32_t * k, const uint32_t * c)
{
    out[0]  = c[0];
    out[1]  = k[0];
    out[2]  = k[1];
    out[3]  = k[2];
    out[4]  = k[3];
    out[5]  = c[1];
    out[6]  = in[0];
    out[7]  = in[1];
    out[8]  = in[2];
    out[9]  = in[3];
    out[10] = c[2];
    out[11] = k[4];
    out[12] = k[5];
    out[13] = k[6];
    out[14] = k[7];
    out[15] = c[3];

    crypto_core_hsalsa20_block(out);

    out[0] += c[0];
    out[1] += k[0];
    out[2] += k[1];
    out[3] += k[2];
    out[4] += k[3];
    out[5] += c[1];
    out[6] += in[0];
    out[7] += in[1];
    out[8] += in[2];
    out[9] += in[3];
    out[10] += c[2];
    out[11] += k[4];
    out[12] += k[5];
    out[13] += k[6];
    out[14] += k[7];
    out[15] += c[3];

    return 0;
}

int crypto_core_salsa20(uint8_t * out, const uint8_t * in, const uint8_t * k, const uint8_t * c)
{
    uint32_t j[16];

    j[0]  = load_littleendian(c + 0);
    j[1]  = load_littleendian(k + 0);
    j[2]  = load_littleendian(k + 4);
    j[3]  = load_littleendian(k + 8);
    j[4]  = load_littleendian(k + 12);
    j[5]  = load_littleendian(c + 4);
    j[6]  = load_littleendian(in + 0);
    j[7]  = load_littleendian(in + 4);
    j[8]  = load_littleendian(in + 8);
    j[9]  = load_littleendian(in + 12);
    j[10] = load_littleendian(c + 8);
    j[11] = load_littleendian(k + 16);
    j[12] = load_littleendian(k + 20);
    j[13] = load_littleendian(k + 24);
    j[14] = load_littleendian(k + 28);
    j[15] = load_littleendian(c + 12);

    crypto_core_salsa20_transform(j);

    store_littleendian(out + 0, j[0]);
    store_littleendian(out + 4, j[1]);
    store_littleendian(out + 8, j[2]);
    store_littleendian(out + 12, j[3]);
    store_littleendian(out + 16, j[4]);
    store_littleendian(out + 20, j[5]);
    store_littleendian(out + 24, j[6]);
    store_littleendian(out + 28, j[7]);
    store_littleendian(out + 32, j[8]);
    store_littleendian(out + 36, j[9]);
    store_littleendian(out + 40, j[10]);
    store_littleendian(out + 44, j[11]);
    store_littleendian(out + 48, j[12]);
    store_littleendian(out + 52, j[13]);
    store_littleendian(out + 56, j[14]);
    store_littleendian(out + 60, j[15]);

    return 0;
}

void crypto_core_hsalsa20_aligned(uint32_t * pOut, uint32_t * pKey, uint32_t * pNonce, uint32_t * pConstant)
{
    pOut[0]  = pConstant[0];
    pOut[1]  = pKey[0];
    pOut[2]  = pKey[1];
    pOut[3]  = pKey[2];
    pOut[4]  = pKey[3];
    pOut[5]  = pConstant[1];
    pOut[6]  = pNonce[0];
    pOut[7]  = pNonce[1];
    pOut[8]  = pNonce[2];
    pOut[9]  = pNonce[3];
    pOut[10] = pConstant[2];
    pOut[11] = pKey[4];
    pOut[12] = pKey[5];
    pOut[13] = pKey[6];
    pOut[14] = pKey[7];
    pOut[15] = pConstant[3];
    crypto_core_hsalsa20_block(pOut);
}

int crypto_core_hsalsa20(uint8_t * out, const uint8_t * in, const uint8_t * k, const uint8_t * c)
{
    uint32_t j[16];

    j[0]  = load_littleendian(c + 0);
    j[1]  = load_littleendian(k + 0);
    j[2]  = load_littleendian(k + 4);
    j[3]  = load_littleendian(k + 8);
    j[4]  = load_littleendian(k + 12);
    j[5]  = load_littleendian(c + 4);
    j[6]  = load_littleendian(in + 0);
    j[7]  = load_littleendian(in + 4);
    j[8]  = load_littleendian(in + 8);
    j[9]  = load_littleendian(in + 12);
    j[10] = load_littleendian(c + 8);
    j[11] = load_littleendian(k + 16);
    j[12] = load_littleendian(k + 20);
    j[13] = load_littleendian(k + 24);
    j[14] = load_littleendian(k + 28);
    j[15] = load_littleendian(c + 12);

    crypto_core_hsalsa20_block(j);

    store_littleendian(out + 0, j[0]);
    store_littleendian(out + 4, j[5]);
    store_littleendian(out + 8, j[10]);
    store_littleendian(out + 12, j[15]);
    store_littleendian(out + 16, j[6]);
    store_littleendian(out + 20, j[7]);
    store_littleendian(out + 24, j[8]);
    store_littleendian(out + 28, j[9]);

    return 0;
}

const UN_128bitValue g_salsa20_sigma = {{'e', 'x', 'p', 'a', 'n', 'd', ' ', '3', '2', '-', 'b', 'y', 't', 'e', ' ', 'k'}};

int crypto_stream_salsa20(uint8_t * c, unsigned clen, const uint8_t * n, const uint8_t * k)
{
    UN_128bitValue in;
    UN_512bitValue block;
    UN_256bitValue key;
    uint64_t i;
    unsigned int u;

    if (!clen)
    {
        return 0;
    }

    for (i = 0; i < 32; ++i)
    {
        key.as_uint8_t[i] = k[i];
    }

    for (i = 0; i < 8; ++i)
    {
        in.as_uint8_t[i] = n[i];
    }

    for (i = 8; i < 16; ++i)
    {
        in.as_uint8_t[i] = 0;
    }

    while (clen >= 64)
    {
        crypto_core_salsa20_aligned((uint32_t *)c, in.as_uint32_t, key.as_uint32_t, g_salsa20_sigma.as_uint32_t);

        //        crypto_core_salsa20(c, in.as_uint8_t, key.as_uint8_t, sigma.as_uint8_t);

        u = 1;

        for (i = 8; i < 16; ++i)
        {
            u += (unsigned int)in.as_uint8_t[i];
            in.as_uint8_t[i] = (uint8_t)u;
            u >>= 8;
        }
        clen -= 64;
        c += 64;
    }

    if (clen)
    {
        crypto_core_salsa20(block.as_uint8_t, in.as_uint8_t, key.as_uint8_t, g_salsa20_sigma.as_uint8_t);

        for (i = 0; i < clen; ++i)
        {
            c[i] = block.as_uint8_t[i];
        }
    }
    return 0;
}

int crypto_stream_salsa20_xor(uint8_t * c, const uint8_t * m, unsigned mlen, const uint8_t * n, const uint8_t * k)
{
    uint8_t in[16];
    uint8_t block[64];
    uint64_t i;
    unsigned int u;

    if (!mlen)
    {
        return 0;
    }

    for (i = 0; i < 8; ++i)
    {
        in[i] = n[i];
    }

    for (i = 8; i < 16; ++i)
    {
        in[i] = 0;
    }

    while (mlen >= 64)
    {
        crypto_core_salsa20(block, in, k, g_salsa20_sigma.as_uint8_t);

        for (i = 0; i < 64; ++i)
        {
            c[i] = m[i] ^ block[i];
        }
        u = 1;

        for (i = 8; i < 16; ++i)
        {
            u += (unsigned int)in[i];
            in[i] = (uint8_t)u;
            u >>= 8;
        }
        mlen -= 64;
        c += 64;
        m += 64;
    }

    if (mlen)
    {
        crypto_core_salsa20(block, in, k, g_salsa20_sigma.as_uint8_t);

        for (i = 0; i < mlen; ++i)
        {
            c[i] = m[i] ^ block[i];
        }
    }
    return 0;
}

int crypto_stream_salsa20_xor_chunked_init(salsa20_xor_chunked *context, const uint8_t *n,const uint8_t *k)
{
    uint32_t i;

    for (i = 0; i < 32; ++i)
    {
        ((uint8_t *)(context->key))[i] = k[i];
    }

    for (i = 0; i < 8; ++i)
    {
        ((uint8_t *)(context->in))[i] = n[i];
    }

    for (i = 8; i < 16; ++i)
    {
        ((uint8_t *)(context->in))[i] = 0;
    }

    context->chunk_bytes_left = 0;

    return 0;
}

int crypto_stream_salsa20_xor_chunked (salsa20_xor_chunked *context, uint8_t *c,const uint8_t *m,unsigned int d)
{
    uint32_t i;
    unsigned int u;
    UN_512bitValue *block;
    uint8_t *in;

    if (!d)
    {
        return 0;
    }

    block = (UN_512bitValue *)(context->block);
    in = (uint8_t *)(context->in);

    if (context->chunk_bytes_left)
    {
        uint32_t e;
        uint32_t offs;

        e = context->chunk_bytes_left;
        offs = 64 - e;
        if (e > d)
        {
            e = d;
        }
        
        for (i = 0; i < e; ++i)
        {
            c[i] = m[i] ^ block->as_uint8_t[offs+i];
        }
        
        c += e;
        m += e;
        d -= e;
        context->chunk_bytes_left -= e;
    }

    while (d >= 64)
    {
        crypto_core_salsa20_aligned(block->as_uint32_t, context->in, context->key, g_salsa20_sigma.as_uint32_t);

        for (i = 0; i < 64; ++i)
        {
            c[i] = m[i] ^ block->as_uint8_t[i];
        }

        u = 1;

        for (i = 8; i < 16; ++i)
        {
            u += (unsigned int)in[i];
            in[i] = (uint8_t)u;
            u >>= 8;
        }

        d -= 64;
        c += 64;
        m += 64;
    }

    if (d)
    {
        context->chunk_bytes_left = 64 - d;

        crypto_core_salsa20_aligned(block->as_uint32_t, context->in, context->key, g_salsa20_sigma.as_uint32_t);

        for (i = 0; i < d; ++i)
        {
            c[i] = m[i] ^ block->as_uint8_t[i];
        }

        u = 1;

        for (i = 8; i < 16; ++i)
        {
            u += (unsigned int)in[i];
            in[i] = (uint8_t)u;
            u >>= 8;
        }
    }

    return 0;
}

int crypto_stream_xsalsa20(uint8_t * c, unsigned int d, const uint8_t * n, const uint8_t * k)
{
    uint8_t s[32];
    crypto_core_hsalsa20(s, n, k, g_salsa20_sigma.as_uint8_t);
    return crypto_stream_salsa20(c, d, n + 16, s);
}

int crypto_stream_xsalsa20_xor(uint8_t * c, const uint8_t * m, unsigned int d, const uint8_t * n, const uint8_t * k)
{
    uint8_t s[32];
    crypto_core_hsalsa20(s, n, k, g_salsa20_sigma.as_uint8_t);
    return crypto_stream_salsa20_xor(c, m, d, n + 16, s);
}
