#include "terminal.h"

#include "stm32h7xx_hal.h"
#include "memory.h"

static __attribute__((section(".uart_handle"))) UART_HandleTypeDef s_huart6;
static __attribute__((section(".uart_handle"))) DMA_HandleTypeDef s_hdma1_stream0;
#define WRITE_PTR (s_terminal_rx_buffer + sizeof(s_terminal_rx_buffer) - __HAL_DMA_GET_COUNTER(&s_hdma1_stream0))

#ifdef CORE_CM4
static uint8_t s_terminal_rx_buffer[256];
static uint8_t* s_terminal_read_ptr = s_terminal_rx_buffer;
#endif

Status terminal_init() {
    #ifdef CORE_CM4
    return STATUS_ERROR; // Should only be init on M7 core
    #else

    GPIO_InitTypeDef gpio_init = {
        .Pin = GPIO_PIN_9 | GPIO_PIN_14,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = GPIO_AF7_USART6,
    };
    HAL_GPIO_Init(GPIOG, &gpio_init);

    __HAL_RCC_USART6_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    memset(&s_huart6, 0, sizeof(s_huart6));
    memset(&s_hdma1_stream0, 0, sizeof(s_hdma1_stream0));

    s_huart6.Init.BaudRate = 115200;
    s_huart6.Init.WordLength = UART_WORDLENGTH_8B;
    s_huart6.Init.StopBits = UART_STOPBITS_1;
    s_huart6.Init.Parity = UART_PARITY_NONE;
    s_huart6.Init.Mode = UART_MODE_TX_RX;
    s_huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    s_huart6.Init.OverSampling = UART_OVERSAMPLING_16;

    s_huart6.Instance = USART6;

    if (HAL_UART_Init(&s_huart6) != HAL_OK) {
        return STATUS_ERROR;
    }

    s_hdma1_stream0.Init.Request = DMA_REQUEST_USART6_RX;
    s_hdma1_stream0.Init.Direction = DMA_PERIPH_TO_MEMORY;
    s_hdma1_stream0.Init.PeriphInc = DMA_PINC_DISABLE;
    s_hdma1_stream0.Init.MemInc = DMA_MINC_ENABLE;
    s_hdma1_stream0.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    s_hdma1_stream0.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    s_hdma1_stream0.Init.Mode = DMA_CIRCULAR;
    s_hdma1_stream0.Init.Priority = DMA_PRIORITY_LOW;
    s_hdma1_stream0.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    s_hdma1_stream0.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    s_hdma1_stream0.Init.MemBurst = DMA_MBURST_SINGLE;
    s_hdma1_stream0.Init.PeriphBurst = DMA_PBURST_SINGLE;

    s_hdma1_stream0.Instance = DMA1_Stream0;

    if (HAL_DMA_Init(&s_hdma1_stream0) != HAL_OK) {
        return STATUS_ERROR;
    }

    __HAL_LINKDMA(&s_huart6, hdmarx, s_hdma1_stream0);

    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
    HAL_NVIC_SetPriority(USART6_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART6_IRQn);

    return STATUS_OK;

    #endif
}

Status terminal_rx_start() {
    #ifndef CORE_CM4
    return STATUS_ERROR; // Should only be called on M4 core
    #else

    s_huart6.pRxBuffPtr = s_terminal_rx_buffer;
    s_huart6.RxXferSize = sizeof(s_terminal_rx_buffer);
    s_huart6.RxXferCount = sizeof(s_terminal_rx_buffer);

    if (HAL_UART_Receive_DMA(&s_huart6, s_terminal_rx_buffer, sizeof(s_terminal_rx_buffer)) != HAL_OK) {
        return STATUS_ERROR;
    }

    return STATUS_OK;

    #endif
}

int terminal_read(uint8_t* buffer, unsigned int size) {
    #ifndef CORE_CM4
    return -1; // Should only be called on M4 core
    #else

    uint8_t *write_ptr = (uint8_t*)(WRITE_PTR);
    unsigned int bytes_available = (write_ptr >= s_terminal_read_ptr)
        ? (write_ptr - s_terminal_read_ptr)
        : (sizeof(s_terminal_rx_buffer) + write_ptr - s_terminal_read_ptr);

    if (bytes_available == 0) {
        return 0;
    }

    unsigned int btr = (size < bytes_available) ? size : bytes_available;

    for (unsigned int i = 0; i < btr; i++) {
        buffer[i] = *s_terminal_read_ptr;
        s_terminal_read_ptr++;
        if (s_terminal_read_ptr >= s_terminal_rx_buffer + sizeof(s_terminal_rx_buffer)) {
            s_terminal_read_ptr = s_terminal_rx_buffer;
        }
    }

    return btr;

    #endif
}

void terminal_write(const char* data, unsigned int size) {
    uint32_t start_tick = HAL_GetTick();
    while (HAL_HSEM_Take(0, 0) != HAL_OK)
    {
        if (HAL_GetTick() - start_tick > TERMINAL_SEMAPHORE_MAX_WAIT)
        {
            return; // timeout
        }
    }
    HAL_UART_Transmit(&s_huart6, (uint8_t*)data, size, 100);
    HAL_HSEM_Release(0, 0);
}

extern "C" {
int _write(int file, char* data, int len);
void DMA1_Stream0_IRQHandler();
void USART6_IRQHandler();
}

int _write(int file, char* data, int len) {
    terminal_write(data, len);
    return len;
}

void DMA1_Stream0_IRQHandler() {
    HAL_DMA_IRQHandler(&s_hdma1_stream0);
}

void USART6_IRQHandler() {
    HAL_UART_IRQHandler(&s_huart6);
}