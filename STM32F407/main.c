#include "stm32wrapper.h"
#include <stdio.h>
#include "main.h"

int main(void)
{
    clock_setup();
    gpio_setup();
    usart_setup(115200);
    rng_enable();

    SCS_DEMCR |= SCS_DEMCR_TRCENA;
    DWT_CYCCNT = 0;
    DWT_CTRL |= DWT_CTRL_CYCCNTENA;
    uint8_t result[32];
    int i;
    unsigned int oldcount = DWT_CYCCNT;
    for (i = 0; i < 100; i++) {
        crypto_scalarmult_base_curve25519(result, static_key.as_uint8_t);
    }
    unsigned int newcount = DWT_CYCCNT-oldcount;

    unsigned char output[32];
    sprintf((char *)output, "Cost: %d", newcount);
    send_USART_str(output);



    unsigned char buffer[50];
    uint32_t res;

	send_USART_str((unsigned char*)"Test scalarmult!");

	res = test_curve25519_DH();
	sprintf((char*)buffer, "Test DH(0 correct): %lu", res);
    send_USART_str(buffer);

    res = test_curve25519_DH_TV();
	sprintf((char*)buffer, "Test DH TV(0 correct): %lu", res);
    send_USART_str(buffer);    

    send_USART_str((unsigned char*)"Done!");

    while(1);

    return 0;
}
