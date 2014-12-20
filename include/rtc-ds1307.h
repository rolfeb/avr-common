#ifndef __INCLUDE_RTC_DS1307_H
#define __INCLUDE_RTC_DS1307_H

#include <stdint.h>

/*
 * Standard I2C address for DS1307 Real Time Clock
 */
#define I2C_ADDR_DS1307     0x68

extern void
rtc_set_time_from_ntp(uint32_t ntp_time);

extern uint8_t
rtc_get_time(uint8_t *sec, uint8_t *min, uint8_t *hr, uint8_t *day, uint8_t *month, uint8_t *year, uint8_t *dow);

extern uint8_t
rtc_init(void);

#endif /* __INCLUDE_RTC_DS1307_H */
