#ifndef LOCAL_SALSA20_HEADER_
#define LOCAL_SALSA20_HEADER_

#include <stdint.h>
#include "../include/crypto_stream.h"


extern int crypto_core_hsalsa20 (
    uint8_t *out,
    const uint8_t *in,
    const uint8_t *k,
    const uint8_t *c
    );

#endif // #ifndef LOCAL_SALSA20_HEADER_
