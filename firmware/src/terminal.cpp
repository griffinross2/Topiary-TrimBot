#include "terminal.h"

#include "stm32h7xx_hal.h"

static UART_HandleTypeDef s_huart6;

Status terminal_init() {
    #ifdef CORE_M4
    return STATUS_ERROR; // Should only be init on M7 core
    #endif

    GPIO_InitTypeDef gpio_init = {
        .Pin = GPIO_PIN_9 | GPIO_PIN_14,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = GPIO_AF7_USART6,
    };
    HAL_GPIO_Init(GPIOG, &gpio_init);

    __HAL_RCC_USART6_CLK_ENABLE();

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

    return STATUS_OK;
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
}

int _write(int file, char* data, int len) {
    terminal_write(data, len);
    return len;
}