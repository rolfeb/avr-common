/*
 * Implement DS1820-specific 1-Wire functions
 */
#include "avr-common.h"
#include "one-wire.h"
#include "ds1820.h"

/*
 * Convert T
 */
void
ds1820_cmd_convert_t(void)
{

    onewire_send_byte(OW_CMD_CONVERT_T);
    onewire_wait_until_done();
}

/*
 * Read Scratchpad
 */
void
ds1820_cmd_read_scratchpad(uint8_t *x)
{
    onewire_send_byte(OW_CMD_READ_SCRATCHPAD);

    for (uint8_t i = 0; i < 9; i++)
        x[i] = onewire_recv_byte();
}

/*
 * Get the temperature from the DS1820 sensor.
 * The protocol is described in the Maxim DS1820 data sheet.
 */
void
ds1820_get_temperature(uint8_t *degrees, uint8_t *half)
{
    uint8_t mem[9];

    mem[0] = 0;

    onewire_reset((uint8_t *)0);
    onewire_send_byte(OW_CMD_SKIP_ROM);
    ds1820_cmd_convert_t();

    onewire_reset((uint8_t *)0);
    onewire_send_byte(OW_CMD_SKIP_ROM);
    ds1820_cmd_read_scratchpad(mem);

    *degrees = mem[0] >> 1;

    if (half)
        *half = mem[0] & 0x01 ? 1 : 0;
}
