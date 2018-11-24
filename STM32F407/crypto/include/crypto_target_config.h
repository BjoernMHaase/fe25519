/*                          =======================
  ============================ C/C++ HEADER FILE =============================
                            =======================                      

    \file crypto_target_config.h

    Target specific adaptions. Defines inttypes and

    The target config header is required to provide definitions for the types

    uint8_t ... uint64
    int8_t  ... int64

    and the macros FORCE_INLINE and NO_INLINE

    In the user specific target config header, it is possible to define the symbols


    in order to make the library use user-supplied assembly optimized multiplication functions.

    By defining the symbol

    CRYPTO_TARGET_NO_CACHEMEM

    the library may be requested to implement optimizations that would be insecure
    on systems with cache memory.
 
    \Author: B. Haase, Endress + Hauser Conducta GmbH & Co. KG

    License: CC0 1.0 (http://creativecommons.org/publicdomain/zero/1.0/legalcode)
  ============================================================================*/

#ifndef CRYPTO_TARGET_CONFIG_HEADER_
#define CRYPTO_TARGET_CONFIG_HEADER_

#include "crypto_target_config_cortexM4.h"

#ifndef FORCE_INLINE
#define FORCE_INLINE
#endif

#ifndef NO_INLINE
#define NO_INLINE
#endif

#endif // #ifdef TARGET_CONFIG_HEADER_
