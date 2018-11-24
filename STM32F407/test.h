#ifndef __TEST_H
#define __TEST_H

#include <stdint.h>
#include <stdio.h>
#include "crypto/include/randombytes.h"
#include "crypto/include/crypto_scalarmult.h"
#include "crypto/include/fe25519.h"



int test_curve25519_DH(void);
int test_curve25519_DH_TV(void);
int test_curve25519_static(void);

int test_cswap(void);

// uint32_t fe25519_cswap_asm(fe25519* x0, fe25519* x1, uint32_t R0, uint32_t R1, uint32_t* b);
// #define cswap_rr fe25519_cswap_asm



#endif
