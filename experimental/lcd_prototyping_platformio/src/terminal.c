#include "terminal.h"

#include "stm32f4xx_hal.h"

static UART_HandleTypeDef s_huart3;

Status terminal_init()
{
    GPIO_InitTypeDef gpio_init = {0};
    gpio_init.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &gpio_init);

    __HAL_RCC_USART3_CLK_ENABLE();

    s_huart3.Init.BaudRate = 115200;
    s_huart3.Init.WordLength = UART_WORDLENGTH_8B;
    s_huart3.Init.StopBits = UART_STOPBITS_1;
    s_huart3.Init.Parity = UART_PARITY_NONE;
    s_huart3.Init.Mode = UART_MODE_TX_RX;
    s_huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    s_huart3.Init.OverSampling = UART_OVERSAMPLING_16;

    s_huart3.Instance = USART3;

    if (HAL_UART_Init(&s_huart3) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

void terminal_write(const char *data, unsigned int size)
{
    HAL_UART_Transmit(&s_huart3, (uint8_t *)data, size, 100);
}

int _write(int file, char *data, int len)
{
    terminal_write(data, len);
    return len;
}