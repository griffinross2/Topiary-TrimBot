#pragma once

#include "stdint.h"

// Hardfault debugging utilities from:
// https://interrupt.memfault.com/blog/cortex-m-hardfault-debug#registers-prior-to-exception

#define HALT_IF_DEBUGGING()                                 \
    do {                                                    \
        if ((*(volatile uint32_t*)0xE000EDF0) & (1 << 0)) { \
            __asm("bkpt 1");                                \
        }                                                   \
    } while (0)

typedef struct __attribute__((packed)) ContextStateFrame {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t return_address;
    uint32_t xpsr;
} sContextStateFrame;

#define HARDFAULT_HANDLING_ASM(_x) \
    __asm volatile(                \
        "tst lr, #4 \n"            \
        "ite eq \n"                \
        "mrseq r0, msp \n"         \
        "mrsne r0, psp \n"         \
        "b hardfault_handler_custom \n")
