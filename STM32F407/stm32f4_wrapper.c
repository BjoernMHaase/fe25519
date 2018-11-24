#include "stm32wrapper.h"

// On some STM32F407 boards, such as the one from riscure, the debug uart output
// signals are routed to UART4 at port C 10.
#define OUTPUT_AT_USART2_AND_UART4

/* 24 MHz */
const struct rcc_clock_scale benchmarkclock = {
    .pllm = 8, //VCOin = HSE / PLLM = 1 MHz
    .plln = 192, //VCOout = VCOin * PLLN = 192 MHz
    .pllp = 8, //PLLCLK = VCOout / PLLP = 24 MHz (low to have 0WS)
    .pllq = 4, //PLL48CLK = VCOout / PLLQ = 48 MHz (required for USB, RNG)
    .hpre = RCC_CFGR_HPRE_DIV_NONE,
    .ppre1 = RCC_CFGR_PPRE_DIV_2,
    .ppre2 = RCC_CFGR_PPRE_DIV_NONE,
    .flash_config = FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_LATENCY_0WS,
    .apb1_frequency = 12000000,
    .apb2_frequency = 24000000,
};

void clock_setup(void)
{
    rcc_clock_setup_hse_3v3(&benchmarkclock);
    // rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART2);

#ifdef OUTPUT_AT_USART2_AND_UART4
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_UART4);
#endif

    rcc_periph_clock_enable(RCC_DMA1);
    rcc_periph_clock_enable(RCC_RNG);

    flash_prefetch_enable();
}

void gpio_setup(void)
{
    // Config for USART2
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2 | GPIO3);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO2 | GPIO3);

#ifdef OUTPUT_AT_USART2_AND_UART4
    // Config for UART4
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10);
    gpio_set_af(GPIOC, GPIO_AF8, GPIO10);
#endif

}

void usart_setup(int baud)
{
    usart_set_baudrate(USART2, baud);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

    usart_enable(USART2);

#ifdef OUTPUT_AT_USART2_AND_UART4
    usart_set_baudrate(UART4, baud);
    usart_set_databits(UART4, 8);
    usart_set_stopbits(UART4, USART_STOPBITS_1);
    usart_set_mode(UART4, USART_MODE_TX);
    usart_set_parity(UART4, USART_PARITY_NONE);
    usart_set_flow_control(UART4, USART_FLOWCONTROL_NONE);

    usart_enable(UART4);
#endif

}

void dma_request_setup(void)
{
    dma_stream_reset(DMA1, DMA_STREAM5);

    nvic_enable_irq(NVIC_DMA1_STREAM5_IRQ);

    dma_set_peripheral_address(DMA1, DMA_STREAM5, (uint32_t) &USART2_DR);
    dma_set_transfer_mode(DMA1, DMA_STREAM5, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);

    dma_set_peripheral_size(DMA1, DMA_STREAM5, DMA_SxCR_PSIZE_8BIT);
    dma_set_memory_size(DMA1, DMA_STREAM5, DMA_SxCR_MSIZE_8BIT);

    dma_set_priority(DMA1, DMA_STREAM5, DMA_SxCR_PL_VERY_HIGH);

    dma_disable_peripheral_increment_mode(DMA1, (uint8_t) DMA_SxCR_CHSEL_4);
    dma_enable_memory_increment_mode(DMA1, DMA_STREAM5);

    dma_disable_transfer_error_interrupt(DMA1, DMA_STREAM5);
    dma_disable_half_transfer_interrupt(DMA1, DMA_STREAM5);
    dma_disable_direct_mode_error_interrupt(DMA1, DMA_STREAM5);
    dma_disable_fifo_error_interrupt(DMA1, DMA_STREAM5);
    dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM5);
}

void dma_transmit_setup(void)
{
    dma_stream_reset(DMA1, DMA_STREAM6);

    nvic_enable_irq(NVIC_DMA1_STREAM6_IRQ);

    dma_set_peripheral_address(DMA1, DMA_STREAM6, (uint32_t) &USART2_DR);
    dma_set_transfer_mode(DMA1, DMA_STREAM6, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);

    dma_set_peripheral_size(DMA1, DMA_STREAM6, DMA_SxCR_PSIZE_8BIT);
    dma_set_memory_size(DMA1, DMA_STREAM6, DMA_SxCR_MSIZE_8BIT);

    dma_set_priority(DMA1, DMA_STREAM6, DMA_SxCR_PL_VERY_HIGH);

    dma_disable_peripheral_increment_mode(DMA1, (uint8_t) DMA_SxCR_CHSEL_4);
    dma_enable_memory_increment_mode(DMA1, DMA_STREAM6);

    dma_disable_transfer_error_interrupt(DMA1, DMA_STREAM6);
    dma_disable_half_transfer_interrupt(DMA1, DMA_STREAM6);
    dma_disable_direct_mode_error_interrupt(DMA1, DMA_STREAM6);
    dma_disable_fifo_error_interrupt(DMA1, DMA_STREAM6);
    dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM6);
}

void dma_request(void* buffer, const int datasize)
{
    dma_set_memory_address(DMA1, DMA_STREAM5, (uint32_t) buffer);
    dma_set_number_of_data(DMA1, DMA_STREAM5, datasize);

    dma_channel_select(DMA1, DMA_STREAM5, DMA_SxCR_CHSEL_4);
    dma_enable_stream(DMA1, DMA_STREAM5);
    signal_host();
    usart_enable_rx_dma(USART2);
}

void dma_transmit(const void* buffer, const int datasize)
{
    dma_set_memory_address(DMA1, DMA_STREAM6, (uint32_t) buffer);
    dma_set_number_of_data(DMA1, DMA_STREAM6, datasize);

    dma_channel_select(DMA1, DMA_STREAM6, DMA_SxCR_CHSEL_4);
    dma_enable_stream(DMA1, DMA_STREAM6);
    usart_enable_tx_dma(USART2);
}

int dma_done(void)
{
    return !((DMA1_S5CR | DMA1_S6CR) & DMA_SxCR_EN);
}

void send_USART_str(const unsigned char* in)
{
    int i;
    for(i = 0; in[i] != 0; i++) {
        usart_send_blocking(USART2, in[i]);
    }
    usart_send_blocking(USART2, '\r');
    usart_send_blocking(USART2, '\n');

#ifdef OUTPUT_AT_USART2_AND_UART4
    for(i = 0; in[i] != 0; i++) {
        usart_send_blocking(UART4, in[i]);
    }
    usart_send_blocking(UART4, '\r');
    usart_send_blocking(UART4, '\n');
#endif
}

void send_USART_bytes(const unsigned char* in, int n)
{
    int i;
    for(i = 0; i < n; i++) {
        usart_send_blocking(USART2, in[i]);
    }

#ifdef OUTPUT_AT_USART2_AND_UART4
    for(i = 0; i < n; i++) {
        usart_send_blocking(UART4, in[i]);
    }
#endif
}

void recv_USART_bytes(unsigned char* out, int n)
{
    int i;
    for(i = 0; i < n; i++) {
        out[i] = usart_recv_blocking(USART2);
    }
}

void dma1_stream5_isr(void) {
    usart_disable_rx_dma(USART2);
    dma_clear_interrupt_flags(DMA1, DMA_STREAM5, DMA_TCIF);
    dma_disable_stream(DMA1, DMA_STREAM5);
}

void dma1_stream6_isr(void) {
    usart_disable_tx_dma(USART2);
    dma_clear_interrupt_flags(DMA1, DMA_STREAM6, DMA_TCIF);
    dma_disable_stream(DMA1, DMA_STREAM6);
}

void signal_host(void) {
    usart_send_blocking(USART2, 'x');
#ifdef OUTPUT_AT_USART2_AND_UART4
    usart_send_blocking(UART4, 'x');
#endif
}
