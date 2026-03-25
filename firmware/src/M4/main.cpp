#include "stm32h7xx_hal.h"

#include <stdio.h>

// extern "C" int main(void);

int main(void)
{
    HAL_Init();

    // Main loop
    while (1)
    {
        HAL_Delay(1000);
    }

    return 0;
}