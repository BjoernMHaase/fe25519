/*                          =======================
  ============================ C/C++ HEADER FILE =============================
                            =======================                      

    \Author: Bjoern Haase 

    License: CC0 1.0 (http://creativecommons.org/publicdomain/zero/1.0/legalcode)
  ============================================================================*/


#ifndef MONTGOMERY_REDUCTION_HEADER_
#define MONTGOMERY_REDUCTION_HEADER_

#include "../include/bigint.h"
#include <assert.h>


typedef struct STMontgomeryConstants256_ {
   UN_256bitValue kSquare; 
   UN_256bitValue prime; 
   uint32_t m0inverse;
} STMontgomeryConstants256;

typedef union UNMontgomeryConstants256_ {
   uint8_t as_uint8_t[sizeof(STMontgomeryConstants256)];
   STMontgomeryConstants256 constants;
} UNMontgomeryConstants256;

#if 0
static uint32
montgomery_mac_U32 (uint32_t *inOut,
                    uint32_t valU32,
                    const uint32_t *multiplier,
                    uint32_t carry)
{
    int i;
    uint64 accu = 0;
    uint16_t valH = valU32 >> 16;
    uint16_t valL = (uint16) valU32;

    for (i = 0; i < 8; i ++)
    {
        accu += inOut[i];

        {
            uint32_t lowValue;
            uint32_t highValue;
            {
                uint32_t tmp = multiplier[i];
                highValue = tmp >> 16;
                lowValue = tmp & 0xfffful;
            }

            accu += lowValue * valL;
            accu += ((uint64)(highValue * valL)) << 16;
            accu += ((uint64)(lowValue * valH)) << 16;
            inOut[i] = (uint32)accu;
            accu >>= 32;
            accu += highValue * valH;
        } 
    }
    accu += inOut[8];
    accu += carry;
    inOut[8] = (uint32) accu;

    return accu >> 32;
}

static uint32
montgomery_mac_lowU16 (uint32_t *inOut,
                       uint16_t valU16,
                       const uint32_t *multiplier)
{
    int i;
    uint64 accu = 0;
    for (i = 0; i < 8; i ++)
    {
        accu += inOut[i];

        {
            uint32_t lowValue;
            uint32_t highValue;
            {
                uint32_t tmp = multiplier[i];
                highValue = tmp >> 16;
                lowValue = tmp & 0xfffful;
            }

            accu += lowValue * valU16;
            accu += ((uint64)(highValue * valU16)) << 16;
            inOut[i] = (uint32)accu;
            accu >>= 32;
        } 
    }

    return (uint32) accu;
}

static uint32
montgomery_mac_highU16 (uint32_t *inOut,
                        uint16_t valU16,
                        const uint32_t *multiplier,
                        uint32_t carry)
{
    int i;
    uint64 accu = 0;
    for (i = 0; i < 8; i ++)
    {
        accu += inOut[i];

        {
            uint32_t lowValue;
            uint32_t highValue;
            {
                uint32_t tmp = multiplier[i];
                highValue = tmp >> 16;
                lowValue = tmp & 0xfffful;
            }

            accu += ((uint64) (lowValue * valU16)) << 16;
            inOut[i] = (uint32) accu;
            accu >>= 32;
            accu += (highValue * valU16);
        } 
    }

    accu += inOut[8];
    accu += carry;
    inOut[8] = (uint32) accu;
 
    return (uint32) (accu >> 32);
}


static int32
montgomery_mnac_U16 (uint32_t *inOut,
                     uint16_t valU16,
                     const uint32_t *multiplier)
{
    int i;
    int64 accu = 0;
    for (i = 0; i < 8; i ++)
    {
        accu += inOut[i];

        {
            uint32_t lowValue;
            uint32_t highValue;
            {
                uint32_t tmp = multiplier[i];
                highValue = tmp >> 16;
                lowValue = tmp & 0xfffful;
            }

            accu -= (lowValue * valU16);
            accu -= ((uint64) (highValue * valU16)) << 16;
            inOut[i] = (uint32) accu;
            accu >>= 32;
        } 
    }
    return (int32) accu;
}
#endif

#if 0

static void 
montgomery_reduce (
    UN_256bitValue *out, 
    UN_512bitValue *in, 
    const STMontgomeryConstants256 *pTable)
{
    uint32_t carry = 0;
    uint32_t ctr = 0;
    uint32_t p0 = pTable->prime.as_uint32_t[0];

    for (ctr = 0; ctr < 8; ctr += 1)
    {
        uint32_t inVal = in->as_uint32_t[ctr];
        uint16_t mpyVal16 = (inVal  & 0xffff) * pTable->m0inverse;

        inVal += mpyVal16 * p0;

        inVal *= pTable->m0inverse;
 
        inVal += mpyVal16;

        carry = montgomery_mac_U32 (
           &in->as_uint32_t[ctr],
           inVal,
           pTable->prime.as_uint32_t,
           carry);
    }
    carry = montgomery_mnac_U16 (&in->as_uint32_t[8],carry + 1,pTable->prime.as_uint32_t);
    montgomery_mac_lowU16 (&in->as_uint32_t[8],carry * carry,pTable->prime.as_uint32_t);

    *out = in->as_256_bitValue[1];
}

#else

void 
montgomery_partialReduce (
    UN_512bitValue *in, 
    const STMontgomeryConstants256 *pTable);


void 
montgomery_reduce (
    UN_256bitValue *out, 
    UN_512bitValue *in, 
    const STMontgomeryConstants256 *pTable);

#endif

#endif

