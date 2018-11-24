/*                          =======================
  ============================ C/C++ HEADER FILE =============================
                            =======================                      

    \file scalarmult_25519.c

    Implements curve25519 ECDH.
    Based on code from avr-nacl project (Author: Michael Hutter, Peter Schwabe)
 
    \Author: B. Haase, Endress + Hauser Conducta GmbH & Co. KG

    License: CC0 1.0 (http://creativecommons.org/publicdomain/zero/1.0/legalcode)
  ============================================================================*/

#include "../include/fe25519.h"
#include "../include/sc25519.h"
#include "../include/randombytes.h"
#include "../include/crypto_scalarmult.h"


// Compile switch for configuring
// Use with care: Swapping pointers results in variable time execution if stack resides in external memory.
// Added here mainly for comparison with the results of the AuCPace paper.
//#define DH_SWAP_BY_POINTERS

typedef struct _ST_curve25519ladderstepWorkingState
{
    // The base point in affine coordinates
    fe25519 x0;

    // The two working points p, q, in projective coordinates. Possibly randomized.
    fe25519 xp;
    fe25519 zp;
    fe25519 xq;
    fe25519 zq;

    UN_256bitValue r;
    UN_256bitValue s;

    int nextScalarBitToProcess;
    uint8_t previousProcessedBit;

#ifdef DH_SWAP_BY_POINTERS
    fe25519 *pXp;
    fe25519 *pZp;
    fe25519 *pXq;
    fe25519 *pZq;
#endif

} ST_curve25519ladderstepWorkingState;

// Original static_key
const UN_256bitValue static_key = {{0x80, 0x65, 0x74, 0xba, 0x61, 0x62, 0xcd, 0x58,
                                    0x49, 0x30, 0x59, 0x47, 0x36, 0x16, 0x35, 0xb6,
                                    0xe7, 0x7d, 0x7c, 0x7a, 0x83, 0xde, 0x38, 0xc0,
                                    0x80, 0x74, 0xb8, 0xc9, 0x8f, 0xd4, 0x0a, 0x43}};

inline void
curve25519_ladderstep(
    ST_curve25519ladderstepWorkingState* pState
) __attribute__((always_inline));

inline void
curve25519_ladderstep(
    ST_curve25519ladderstepWorkingState* pState
)
{
    // Implements the "ladd-1987-m-3" differential-addition-and-doubling formulas
    // Source: 1987 Montgomery "Speeding the Pollard and elliptic curve methods of factorization", page 261,
    //         fifth and sixth displays, plus common-subexpression elimination.
    //
    // Notation from the explicit formulas database:
    // (X2,Z2) corresponds to (xp,zp),
    // (X3,Z3) corresponds to (xq,zq)
    // Result (X4,Z4) (X5,Z5) expected in (xp,zp) and (xq,zq)
    //
    // A = X2+Z2; AA = A^2; B = X2-Z2; BB = B^2; E = AA-BB; C = X3+Z3; D = X3-Z3;
    // DA = D*A; CB = C*B; t0 = DA+CB; t1 = t0^2; X5 = Z1*t1; t2 = DA-CB;
    // t3 = t2^2; Z5 = X1*t3; X4 = AA*BB; t4 = a24*E; t5 = BB+t4; Z4 = E*t5 ;
    //
    // Re-Ordered for using less temporaries.

    fe25519 t1, t2;

    #ifdef DH_SWAP_BY_POINTERS
    fe25519 *b1=pState->pXp; fe25519 *b2=pState->pZp;
    fe25519 *b3=pState->pXq; fe25519 *b4=pState->pZq;
    #else
    fe25519 *b1=&pState->xp; fe25519 *b2=&pState->zp;
    fe25519 *b3=&pState->xq; fe25519 *b4=&pState->zq;
    #endif

    fe25519 *b5= &t1; fe25519 *b6=&t2;

    fe25519_add(b5,b1,b2); // A = X2+Z2
    fe25519_sub(b6,b1,b2); // B = X2-Z2
    fe25519_add(b1,b3,b4); // C = X3+Z3
    fe25519_sub(b2,b3,b4); // D = X3-Z3
    fe25519_mul(b3,b2,b5); // DA= D*A
    fe25519_mul(b2,b1,b6); // CB= C*B
    fe25519_add(b1,b2,b3); // T0= DA+CB
    fe25519_sub(b4,b3,b2); // T2= DA-CB
    fe25519_square(b3,b1); // X5==T1= T0^2
    fe25519_square(b1,b4); // T3= t2^2
    fe25519_mul(b4,b1,&pState->x0); // Z5=X1*t3
    fe25519_square(b1,b5); // AA=A^2
    fe25519_square(b5,b6); // BB=B^2
    fe25519_sub(b2,b1,b5); // E=AA-BB
    fe25519_mul(b1,b5,b1); // X4= AA*BB
#ifdef CRYPTO_HAS_ASM_COMBINED_MPY121666ADD_FE25519
    fe25519_mpy121666add(b6,b5,b2);
#else
    fe25519_mpyWith121666 (b6,b2); // T4 = a24*E
    fe25519_add(b6,b6,b5); // T5 = BB + t4
#endif
    fe25519_mul(b2,b6,b2); // Z4 = E*t5
}

static void
curve25519_cswap(
    ST_curve25519ladderstepWorkingState* state,
    uint8_t                                b
)
{
    #ifdef DH_SWAP_BY_POINTERS
    swapPointersConditionally ((void **) &state->pXp,(void **) &state->pXq,b);
    swapPointersConditionally ((void **) &state->pZp,(void **) &state->pZq,b);
    #else
    fe25519_cswap (&state->xp, &state->xq,b);
    fe25519_cswap (&state->zp, &state->zq,b);
    #endif
}

int
crypto_scalarmult_curve25519(
    uint8_t*       r,
    const uint8_t* s,
    const uint8_t* p
)
{
    ST_curve25519ladderstepWorkingState state;
    uint8_t i;


    // Prepare the scalar within the working state buffer.
    for (i = 0; i < 32; i++)
    {
        state.s.as_uint8_t [i] = s[i];
    }

    state.s.as_uint8_t [0] &= 248; 
    state.s.as_uint8_t [31] &= 127;
    state.s.as_uint8_t [31] |= 64;

    // Copy the affine x-axis of the base point to the state.
    fe25519_unpack (&state.x0, p);

    // Prepare the working points within the working state struct.
    fe25519_setone (&state.zq);
    fe25519_cpy (&state.xq, &state.x0);

    fe25519_setone(&state.xp);
    fe25519_setzero(&state.zp);


    state.nextScalarBitToProcess = 254;

#ifdef DH_SWAP_BY_POINTERS
    // we need to initially assign the pointers correctly.
    state.pXp = &state.xp;
    state.pZp = &state.zp;
    state.pXq = &state.xq;
    state.pZq = &state.zq;
#endif

    state.previousProcessedBit = 0;

    // Process all the bits except for the last three where we explicitly double the result.
    while (state.nextScalarBitToProcess >= 0) 
    {
        uint8_t byteNo = (uint8_t)(state.nextScalarBitToProcess >> 3);
        uint8_t bitNo = (uint8_t)(state.nextScalarBitToProcess & 7);
        uint8_t bit;
        uint8_t swap;

        bit = 1 & (state.s.as_uint8_t [byteNo] >> bitNo);
        swap = bit ^ state.previousProcessedBit;
        state.previousProcessedBit = bit;
        curve25519_cswap(&state, swap);
        curve25519_ladderstep(&state);
        state.nextScalarBitToProcess --;
    }

    curve25519_cswap(&state,state.previousProcessedBit);

    // optimize for stack usage.
    fe25519_invert_useProvidedScratchBuffers (&state.zp, &state.zp, &state.xq, &state.zq, &state.x0);    
    fe25519_mul(&state.xp, &state.xp, &state.zp);
    fe25519_reduceCompletely(&state.xp);

    fe25519_pack (r, &state.xp);

    return 0;
}

const uint8_t g_basePointCurve25519[32] =
{
    9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

int
crypto_scalarmult_base_curve25519(
    uint8_t*       q,
    const uint8_t* n
)
{
    return crypto_scalarmult_curve25519(q, n, g_basePointCurve25519);
}

