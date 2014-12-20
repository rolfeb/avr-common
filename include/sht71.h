#ifndef __INCLUDE_SHT71_H
#define __INCLUDE_SHT71_H

#include "avr-common.h"

typedef struct
{
    uint16_t    temperature;
    uint16_t    humidity;
}
    sht71_measure_t;

extern void
sht71_init(gpio_line_t *sck_line, gpio_line_t *data_line);

extern uint8_t
sht71_read_status(void);

extern void
sht71_write_status(uint8_t v);

extern void
sht71_soft_reset(void);

extern void
sht71_read_values(sht71_measure_t *values);

#endif /* __INCLUDE_SHT71_H */
