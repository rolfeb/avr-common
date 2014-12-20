#ifndef __INCLUDE_DS1820_H
#define __INCLUDE_DS1820_H

#define OW_CMD_CONVERT_T        0x44
#define OW_CMD_READ_SCRATCHPAD  0xbe

extern void
ds1820_cmd_convert_t(void);

extern void
ds1820_cmd_read_scratchpad(uint8_t *x);

extern void
ds1820_get_temperature(uint8_t *degrees, uint8_t *half);

#endif /* __INCLUDE_DS1820_H */
