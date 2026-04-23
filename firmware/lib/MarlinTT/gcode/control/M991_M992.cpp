#include "../gcode.h"
#include "../../module/endstops.h"
#include "board.h"
#include "marlin_wrapper.h"
#include "gpio/gpio.h"

/**
 * M991: Enable cutter
 */
void GcodeSuite::M991() { 
    if (marlin_wrapper_is_alive()) {
        gpio_write(PIN_CUTTER, GPIO_HIGH);
    }
 }

/**
 * M992: Disable cutter
 */
void GcodeSuite::M992() { 
    gpio_write(PIN_CUTTER, GPIO_LOW);
}
