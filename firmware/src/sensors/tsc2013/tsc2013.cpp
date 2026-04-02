#include "tsc2013/tsc2013.h"

#include "board.h"
#include "gpio/gpio.h"
#include "i2c/i2c.h"
#include "stm32h7xx_hal.h"

#define TSC2013_MAX_MULTI_READ 16

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

// static EXTI_HandleTypeDef s_hexti;

static constexpr struct {
    float ax, bx, cx;
    float ay, by, cy;
} s_calibration_params = {
    0.852788f, -0.00406077f, -41.4795f,
    -0.00769267f, -0.558612f, 515.039f,
};

// static void (*s_tsc2013_touch_callback)(int tx, int ty) = nullptr;

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
    if (count > TSC2013_MAX_MULTI_READ)
    {
        return STATUS_PARAMETER_ERROR;
    }

    // 1st byte: control byte (bit 7 = 0 for registers, bit 3-6 are register address, bit 0 = 1 for write)
    uint8_t buf = (reg & 0xF) << 3 | 0x1;

    // Send register address
    if (i2c_write(&s_i2c_dev, &buf, 1) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    // Read register values
    uint8_t rx_buf[2*TSC2013_MAX_MULTI_READ];
    if (i2c_read(&s_i2c_dev, rx_buf, 2 * count) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    for (size_t i = 0; i < count; i++)
    {
        

        value[i] = (rx_buf[2*i] << 8) | rx_buf[2*i + 1];
    }

    return STATUS_OK;
}

Status tsc2013_start_conversion()
{
    // Control byte: bit 7 = 1 for conversion command
    uint8_t buf = 0x80;
    return i2c_write(&s_i2c_dev, &buf, 1);
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
    // Bit 15-14    - PINTS = 00 (AND of PENIRQ and DAV (goes high when data should be read))
    // Bit 13-12    - 01 (M=1)
    // Bit 11-10    - 10 (W=2)
    // Bit 9-5      - Reserved, set to 0
    // Bit 4-1      - MAVE = 1111 (MAV filter enabled)
    // Bit 0        - Reserved, set to 0
    constexpr uint16_t cfr2_val = 0x181E;

    if (tsc2013_write_reg(TSC2013_REG_CFR2, cfr2_val) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    if (tsc2013_read_reg(TSC2013_REG_CFR2, &readback_val) != STATUS_OK || readback_val != cfr2_val)
    {
        TRACE_PRINTF("TSC2013 CFR2 readback failed: expected 0x%04X, got 0x%04X\n", cfr2_val, readback_val);
        return STATUS_ERROR;
    }

    // Start conversion
    if (tsc2013_start_conversion() != STATUS_OK)
    {
        TRACE_PRINTF("TSC2013 start conversion failed\n");
        return STATUS_ERROR;
    }

    // Configure touch interrupt
    // gpio_mode(PIN_TS_INT, GPIO_INPUT);

    // s_hexti.Line = EXTI_LINE_3;
    // EXTI_ConfigTypeDef exti_config = {
    //     .Line = EXTI_LINE_3,
    //     .Mode = EXTI_MODE_INTERRUPT,
    //     .Trigger = EXTI_TRIGGER_RISING,
    //     .GPIOSel = EXTI_GPIOF,
    //     .PendClearSource = EXTI_D3_PENDCLR_SRC_NONE,
    // };
    // HAL_EXTI_SetConfigLine(&s_hexti, &exti_config);

    // NVIC_SetPriority(EXTI3_IRQn, 1);
    // NVIC_EnableIRQ(EXTI3_IRQn);

    return STATUS_OK;
}

bool tsc2013_is_touched() {
    uint16_t status = 0;
    if (tsc2013_read_reg(TSC2013_REG_CFR0, &status) != STATUS_OK)
    {
        return false;
    }

    if (status & 0x8000)
    {
        return true;
    }

    return false;
}

bool tsc2013_is_data_ready() {
    uint16_t status = 0;
    if (tsc2013_read_reg(TSC2013_REG_STATUS, &status) != STATUS_OK)
    {
        return false;
    }

    if (status & 0xF8)
    {
        return true;
    }

    return false;
}

Status tsc2013_read_touch(uint16_t *x, uint16_t *y, uint16_t *z) {
    uint16_t values[8];
    if (tsc2013_multi_read_reg(TSC2013_REG_X1, values, 8) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    uint16_t X1 = values[0];
    uint16_t X2 = values[1];
    uint16_t Y1 = values[2];
    uint16_t Y2 = values[3];
    // uint16_t IX = values[4];
    // uint16_t IY = values[5];
    // uint16_t Z1 = values[6];
    // uint16_t Z2 = values[7];

    if (X1 >= X2)
	{
		*x = X2 + ((X1 - X2) >> 1);
	}
	else
	{
		*x = X1 + ((X2 - X1) >> 1);
	}

	if (Y1 >= Y2)
	{
		*y = Y2 + ((Y1 - Y2) >> 1);
	}
	else
	{
		*y = Y1 + ((Y2 - Y1) >> 1);
	}

	*x = 1024 - *x;
	*y = 1024 - *y;
    *z = 0;

    // Apply calibration
    *x = s_calibration_params.ax * (*x) + s_calibration_params.bx * (*y) + s_calibration_params.cx;
    *y = s_calibration_params.ay * (*x) + s_calibration_params.by * (*y) + s_calibration_params.cy;

    return STATUS_OK;
}

// void tsc2013_set_touch_callback(void (*callback)(int tx, int ty)) {
//     s_tsc2013_touch_callback = callback;
// }

// extern "C"
// {
//     void EXTI3_IRQHandler(void);
// }

// void EXTI3_IRQHandler(void)
// {
//     HAL_EXTI_IRQHandler(&s_hexti);

//     // Make sure data is read (it should be)
//     if (!tsc2013_is_data_ready())
//     {
//         return;
//     }

//     // Read the touch data
//     uint16_t x, y, z;
//     if (tsc2013_read_touch(&x, &y, &z) != STATUS_OK)
//     {
//         return;
//     }

//     // printf("Touch data - X: %u, Y: %u, Z: %u\n", x, y, z);
//     if (s_tsc2013_touch_callback)
//     {
//         s_tsc2013_touch_callback(x, y);
//     }
// }