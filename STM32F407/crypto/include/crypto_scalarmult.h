#ifndef CRYPTO_SCALARMULT_H
#define CRYPTO_SCALARMULT_H

#include <stdint.h>
#include "bigint.h"

#define crypto_scalarmult crypto_scalarmult_curve25519
#define crypto_scalarmult_base crypto_scalarmult_base_curve25519

#define crypto_scalarmult_BYTES 32
#define crypto_scalarmult_SCALARBYTES 32

extern const UN_256bitValue static_key;

int
crypto_scalarmult_curve25519(
    uint8_t*       r,
    const uint8_t* s,
    const uint8_t* p
);

int
crypto_scalarmult_base_curve25519(
    uint8_t*       q,
    const uint8_t* n
);


extern const uint8_t g_basePointCurve25519[32];

#endif
