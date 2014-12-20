#ifndef __INCLUDE_ONE_WIRE_H
#define __INCLUDE_ONE_WIRE_H

#define OW_CMD_SKIP_ROM         0xcc
#define OW_CMD_READ_ROM         0x33
#define OW_CMD_MATCH_ROM        0x55
#define OW_CMD_SEARCH_ROM       0xf0

extern void
onewire_init(volatile uint8_t *ddr, volatile uint8_t *out,
    volatile uint8_t *in, uint8_t line);

extern void
onewire_reset(uint8_t *x);

extern void
onewire_send_bit(uint8_t x);

extern void
onewire_send_byte(uint8_t x);

extern uint8_t
onewire_recv_bit(void);

extern uint8_t
onewire_recv_byte(void);

extern void
onewire_wait_until_done(void);

#if AVR_FEATURE_ONEWIRE_ENABLE_SEARCHROM
extern void
onewire_scan_init(void);

extern uint8_t
onewire_scan_next(uint64_t *x);

extern uint8_t
onewire_scan_first(uint64_t *serial);
#endif

#if AVR_FEATURE_ONEWIRE_ENABLE_MATCHROM
extern void
onewire_cmd_match_rom(uint64_t rom);
#endif

#endif /* __INCLUDE_ONE_WIRE_H */
