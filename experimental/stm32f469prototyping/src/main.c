#define RCC 0x40023800
#define GPIOG 0x40021800

int main() {
    // Enable clock for GPIOG
    *(volatile unsigned int *)(RCC + 0x30) |= (1 << 6);
    // Set PG6 as output
    *(volatile unsigned int *)(GPIOG + 0x00) &= ~(0b11 << (6 * 2));
    *(volatile unsigned int *)(GPIOG + 0x00) |= (0b01 << (6 * 2));
    while (1) {
        // Set PG6 high
        *(volatile unsigned int *)(GPIOG + 0x18) = (1 << 6);
        for (volatile int i = 0; i < 1000000; i++) { asm("nop"); }
        // Set PG6 low
        *(volatile unsigned int *)(GPIOG + 0x18) = (1 << (6 + 16));
        for (volatile int i = 0; i < 1000000; i++) { asm("nop"); }
    }

    return 0;
}