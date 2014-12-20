#ifndef __INCLUDE_AVR_COMMON_H
#define __INCLUDE_AVR_COMMON_H

#include <stdint.h>

/*
 * Include optional module configuration file (module-cfg.h)
 */
#ifdef MODULE_CFG_H
#include MODULE_CFG_H
#endif /* MODULE_CFG_H */

/*
 * Various useful macros and typedefs
 */
#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

/*
 * A structure for defining a GPIO line
 */
typedef struct
{
    volatile uint8_t    *ddr;   /* The DDR register ID */
    volatile uint8_t    *p_out; /* The PORT register ID */
    volatile uint8_t    *p_in;  /* The PIN register ID */
    uint8_t             line;   /* The Line ID */
}
    gpio_line_t;

#endif /* __INCLUDE_AVR_COMMON_H */
