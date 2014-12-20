#ifndef __INCLUDE_HP30_H
#define __INCLUDE_HP30_H

#include "avr-common.h"

typedef struct
{
    uint32_t    pressure;
    uint32_t    temperature;
}
    hp30_measure_t;

extern void
hp30_init(uint8_t volatile *port, uint8_t volatile *ddr, uint8_t pin);

extern void
hp30_read_values(hp30_measure_t *values);

#endif /* __INCLUDE_HP30_H */

