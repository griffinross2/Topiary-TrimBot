#include "tsc2013/tsc2013.h"

#include "board.h"
#include "gpio/gpio.h"
#include "i2c/i2c.h"
#include "stm32h7xx_hal.h"

uint8_t operator"" _u8(unsigned long long x)
{
    return x;
}

uint16_t operator"" _u16(unsigned long long x)
{
    return x;
}

constexpr static I2cDevice s_i2c_dev = {
    .address = TSC2013_I2C_ADDRESS,
    .clk = I2C_SPEED_STANDARD,
    .periph = P_I2C2,
    .scl = PIN_TS_SCL,
    .sda = PIN_TS_SDA,
};

static EXTI_HandleTypeDef s_hexti;

Status tsc2013_write_reg(uint8_t reg, uint16_t value)
{
    // 1st byte: control byte (bit 7 = 0 for registers, bit 3-6 are register address, bit 0 = 0 for write)
    uint8_t buf[3] = {static_cast<uint8_t>((reg & 0xF) << 3), static_cast<uint8_t>((value >> 8) & 0xFF), static_cast<uint8_t>(value & 0xFF)};

    return i2c_write(&s_i2c_dev, buf, 3);
}

Status tsc2013_read_reg(uint8_t reg, uint16_t *value)
{
    // 1st byte: control byte (bit 7 = 0 for registers, bit 3-6 are register address, bit 0 = 1 for write)
    uint8_t buf = (reg & 0xF) << 3 | 0x1;

    // Send register address
    if (i2c_write(&s_i2c_dev, &buf, 1) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    // Read register value
    uint8_t rx_buf[2];
    if (i2c_read(&s_i2c_dev, rx_buf, 2) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    *value = (rx_buf[0] << 8) | rx_buf[1];

    return STATUS_OK;
}

Status tsc2013_multi_read_reg(uint8_t reg, uint16_t *value, size_t count)
{
    // 1st byte: control byte (bit 7 = 0 for registers, bit 3-6 are register address, bit 0 = 1 for write)
    uint8_t buf = (reg & 0xF) << 3 | 0x1;

    // Send register address
    if (i2c_write(&s_i2c_dev, &buf, 1) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    // Read register values
    uint8_t rx_buf[2];
    for (size_t i = 0; i < count; i++)
    {
        if (i2c_read(&s_i2c_dev, rx_buf, 2) != STATUS_OK)
        {
            return STATUS_ERROR;
        }

        value[i] = (rx_buf[0] << 8) | rx_buf[1];
    }

    return STATUS_OK;
}

Status tsc2013_init()
{
    // Reset
    gpio_write(PIN_TS_NRST, GPIO_LOW);
    HAL_Delay(10);
    gpio_write(PIN_TS_NRST, GPIO_HIGH);
    HAL_Delay(10);

    uint16_t readback_val = 0;

    // CFR0:
    // Bit 15       - PSM = 1 (TSC controls converter)
    // Bit 14       - STS = 0 (normal operation)
    // Bit 13       - RM = 0 (10 bit res)
    // Bit 12-11    - CL = 00 (4 MHz)
    // Bit 10-8     - PV = 000 (0 us delay time)
    // Bit 7-5      - PR = 000 (20 us precharge time)
    // Bit 4-2      - SN = 000 (32 us sense time)
    // Bit 1        - DTW = 0 (no detection in wait)
    // Bit 0        - LSM = 0 (no extra sampling time)
    constexpr uint16_t cfr0_val = 0x8000;

    if (tsc2013_write_reg(TSC2013_REG_CFR0, cfr0_val) != STATUS_OK)
    {
        TRACE_PRINTF("TSC2013 CFR0 write failed\n");
        return STATUS_ERROR;
    }

    // No readback on CFR0

    // CFR1:
    // Bit 15-3     - Reserved, set to 0
    // Bit 2-0      - BTD = 000 (normal)
    constexpr uint16_t cfr1_val = 0x0000;

    if (tsc2013_write_reg(TSC2013_REG_CFR1, cfr1_val) != STATUS_OK)
    {
        TRACE_PRINTF("TSC2013 CFR1 write failed\n");
        return STATUS_ERROR;
    }

    if (tsc2013_read_reg(TSC2013_REG_CFR1, &readback_val) != STATUS_OK || readback_val != 0x0000)
    {
        TRACE_PRINTF("TSC2013 CFR1 readback failed: expected 0x%04X, got 0x%04X\n", cfr1_val, readback_val);
        return STATUS_ERROR;
    }

    // CFR2:
    // Bit 15-14    - PINTS = 10 (interrupt on PENIRQ)
    // Bit 13-10    - 0000 (preprocessing disabled)
    // Bit 9-5      - Reserved, set to 0
    // Bit 4-1      - MAVE = 0000 (MAV filter disabled)
    // Bit 0        - Reserved, set to 0
    constexpr uint16_t cfr2_val = 0x8000;

    if (tsc2013_write_reg(TSC2013_REG_CFR2, cfr2_val) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    if (tsc2013_read_reg(TSC2013_REG_CFR2, &readback_val) != STATUS_OK || readback_val != 0x8000)
    {
        TRACE_PRINTF("TSC2013 CFR2 readback failed: expected 0x%04X, got 0x%04X\n", cfr2_val, readback_val);
        return STATUS_ERROR;
    }

    // Configure touch interrupt
    s_hexti.Line = EXTI_LINE_3;
    EXTI_ConfigTypeDef exti_config = {
        .Line = EXTI_LINE_3,
        .Mode = EXTI_MODE_INTERRUPT,
        .Trigger = EXTI_TRIGGER_FALLING,
        .GPIOSel = EXTI_GPIOF,
        .PendClearSource = EXTI_D3_PENDCLR_SRC_NONE,
    };
    HAL_EXTI_SetConfigLine(&s_hexti, &exti_config);

    NVIC_SetPriority(EXTI3_IRQn, 1);
    NVIC_EnableIRQ(EXTI3_IRQn);

    return STATUS_OK;
}

void EXTI3_IRQHandler(void)
{
    HAL_EXTI_IRQHandler(&s_hexti);

    printf("Touch interrupt triggered\n");
}