// 
// randombytes implementation for STM32F407 hardware
//

#include <stdint.h>
#include <libopencm3/stm32/rng.h>
#include "../include/randombytes.h"


void randombytes(unsigned char *x,unsigned long long xlen)
{
    union
    {
        unsigned char aschar[4];
        uint32_t asint;
    } random;

    while (xlen > 4)
    {
        random.asint = rng_get_random_blocking();
        *x++ = random.aschar[0];
        *x++ = random.aschar[1];
        *x++ = random.aschar[2];
        *x++ = random.aschar[3];
        xlen -= 4;
    }
    if (xlen > 0)
    {
        for (random.asint = rng_get_random_blocking(); xlen > 0; --xlen)
        {
            *x++ = random.aschar[xlen - 1];
        }
    }
}
