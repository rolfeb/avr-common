#ifndef __INCLUDE_CLOCK_H
#define __INCLUDE_CLOCK_H

#include <stdint.h>

extern void
clock_init(void);

extern void
clock_increment(void);

extern uint32_t
clock_current_time(void);

#endif /* __INCLUDE_CLOCK_H */
