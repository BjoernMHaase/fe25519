#ifndef CRYPTO_STREAM_H
#define CRYPTO_STREAM_H 1

#include<stdint.h>

#define crypto_stream crypto_stream_xsalsa20
#define crypto_stream_xor crypto_stream_xsalsa20_xor
#define crypto_stream_KEYBYTES crypto_stream_xsalsa20_KEYBYTES
#define crypto_stream_NONCEBYTES crypto_stream_xsalsa20_NONCEBYTES

#define crypto_stream_xsalsa20_KEYBYTES 32
#define crypto_stream_xsalsa20_NONCEBYTES 24

#define crypto_stream_salsa20_KEYBYTES 32
#define crypto_stream_salsa20_NONCEBYTES 8

#define crypto_stream_salsa20_CHUNKBYTES 64

int
crypto_stream_salsa20 (uint8_t *c,unsigned int d,const uint8_t *n,const uint8_t *k);

int
crypto_stream_salsa20_xor (uint8_t *c,const uint8_t *m,unsigned int d,const uint8_t *n,const uint8_t *k);

int
crypto_stream_xsalsa20 (uint8_t *c,unsigned int d,const uint8_t *n,const uint8_t *k);

int
crypto_stream_xsalsa20_xor (uint8_t *c,const uint8_t *m,unsigned int d,const uint8_t *n,const uint8_t *k);


typedef struct
{
    uint32_t in[(2 * crypto_stream_salsa20_NONCEBYTES) / sizeof(uint32_t)];
    uint32_t key[crypto_stream_salsa20_KEYBYTES / sizeof(uint32_t)];
    uint32_t block[crypto_stream_salsa20_CHUNKBYTES / sizeof(uint32_t)];
    uint32_t chunk_bytes_left;
} salsa20_xor_chunked;

int
crypto_stream_salsa20_xor_chunked_init(salsa20_xor_chunked *context, const uint8_t *n,const uint8_t *k);

int
crypto_stream_salsa20_xor_chunked (salsa20_xor_chunked *context, uint8_t *c,const uint8_t *m,unsigned int d);


#ifdef CRYPTO_HAS_ASM_HSALSA20_BLOCK

#define crypto_core_hsalsa20_block crypto_core_hsalsa20_block_asm

#endif

extern void 
crypto_core_hsalsa20_aligned (uint32_t *pOut,
    uint32_t *pKey,
    uint32_t *pNonce,
    uint32_t *pConstant);

extern void
crypto_core_hsalsa20_block (volatile uint32_t * x);

#endif
