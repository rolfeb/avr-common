/*
 * Link in the required AVR feature modules
 */

#include "avr-features.h"

#if AVR_FEATURE_I2C
#include "i2c.c"
#endif

#if AVR_FEATURE_SPI
#include "spi.c"
#endif

#if AVR_FEATURE_UART0
#include "uart0.c"
#endif

#if AVR_FEATURE_DS1820
#include "ds1820.c"
#endif

#if AVR_FEATURE_DS1307
#include "rtc-ds1307.c"
#endif

#if AVR_FEATURE_LCD2S
#include "lcd2s.c"
#endif

#if AVR_FEATURE_ENC28J60
#include "enc28j60.c"
#endif

#if AVR_FEATURE_ONEWIRE
#include "one-wire.c"
#endif

#if AVR_FEATURE_NWSTACK
#include "nw-stack.c"
#endif

#if AVR_FEATURE_NWSTACK2
#include "nw-stack2.c"
#endif

#if AVR_FEATURE_CLOCK
#include "clock.c"
#endif

#if AVR_FEATURE_TASKS
#include "task.c"
#endif

void
avr_features_init(void)
{
#if AVR_FEATURE_I2C
    i2c_init();
#endif

#if AVR_FEATURE_SPI
    spi_init();
#endif

#if AVR_FEATURE_CLOCK
    clock_init();
#endif

#if AVR_FEATURE_DS1307
    rtc_init();
#endif

#if AVR_FEATURE_TASK
    task_init();
#endif
}
