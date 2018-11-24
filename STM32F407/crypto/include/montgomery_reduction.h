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

