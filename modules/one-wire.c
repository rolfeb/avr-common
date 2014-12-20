/*
 * Module for running the Maxim 1-Wire bus protocol
 */
#include <avr/io.h>
#include <util/delay.h>

#include "avr-common.h"
#include "one-wire.h"

static volatile uint8_t  *ddr_reg;  // the data direction port ID
static volatile uint8_t  *out_reg;  // the I/O port ID
static volatile uint8_t  *in_reg;   // the I/O port ID
static uint8_t  io_line;            // the I/O line 

/*
 * Setup I/O parameters
 */
void
onewire_init(volatile uint8_t *ddr, volatile uint8_t *out,
    volatile uint8_t *in, uint8_t line)
{
    ddr_reg = ddr;
    out_reg = out;
    in_reg = in;
    io_line = line;

    cbi(*ddr_reg, io_line);
    cbi(*out_reg, io_line);   // disable pull-up on io_line
}

/*
 * Reset all devices on the bus
 */
void
onewire_reset(uint8_t *x)
{
    uint8_t detect  = 0;

    // pull io_line low
    sbi(*ddr_reg, io_line);
    cbi(*out_reg, io_line);

    // hold for 480 us
    for (int i = 0; i < 8; i++)
        _delay_us(60);

    // release to tri-state
    cbi(*ddr_reg, io_line);

    // wait 70 us
    _delay_us(70);

    // sample io_line
    if ((*in_reg & (1 << io_line)) == 0)
    {
        detect = 1;

        // wait for io_line to return high
        while ((*in_reg & (1 << io_line)) == 0)
            _delay_us(5);

    }

    // wait 420 us
    for (int i = 0; i < 7; i++)
        _delay_us(60);

    if (x)
        *x = detect;
}

/*
 * Send a single bit on io_line
 *
 * Write 1 slot: pull low for 5us, release to tri-state for 85us
 * Write 0 slot: pull low for 90us, release to tri-state
 */
void
onewire_send_bit(uint8_t x)
{
    // pull io_line low to start write slot
    cbi(*out_reg, io_line);
    sbi(*ddr_reg, io_line);

    if (x)
    {
        _delay_us(6);

        // release to tri-state
        cbi(*ddr_reg, io_line);

        _delay_us(64);
    }
    else
    {
        _delay_us(60);

        // release to tri-state
        cbi(*ddr_reg, io_line);

        _delay_us(10);
    }

}

/*
 * Send a byte on io_line
 */
void
onewire_send_byte(uint8_t x)
{
    int mask    = 1;

    for (int i = 0; i < 8; i++)
    {
        if (x & mask)
            onewire_send_bit(1);
        else
            onewire_send_bit(0);

        mask <<= 1;
    }
}

/*
 * Receive a single bit on io_line
 */
uint8_t
onewire_recv_bit(void)
{
    uint8_t bit = 0;

    // pull io_line low to start read slot
    cbi(*out_reg, io_line);
    sbi(*ddr_reg, io_line);

    _delay_us(6);

    // release to tri-state
    cbi(*out_reg, io_line);
    cbi(*ddr_reg, io_line);

    _delay_us(9);

    // sample io_line
    if ((*in_reg & (1 << io_line)) != 0)
        bit = 1;

    // delay for end of slot
    _delay_us(55);

    return bit;
}

/*
 * Receive a byte on io_line
 */
uint8_t
onewire_recv_byte(void)
{
    uint8_t x       = 0;
    int     mask    = 1;

    for (int i = 0; i < 8; i++)
    {
        if (onewire_recv_bit())
            x |= mask;

        mask <<= 1;
    }

    return x;
}

void
onewire_wait_until_done(void)
{
    while (onewire_recv_bit() == 0)
        _delay_us(60);
}

#if AVR_FEATURE_ONEWIRE_ENABLE_SEARCHROM

/*
 * Routines to perform the Search ROM function
 */
static uint8_t  done_flag;
static uint8_t  last_discrepancy;

void
onewire_scan_init(void)
{
    last_discrepancy = 0;
    done_flag = 0;
}

uint8_t
onewire_scan_next(uint64_t *x)
{
    uint64_t    serial  = 0;
    uint64_t    mask    = 0x1;
    uint8_t     detect;
    uint8_t     bit;    // starts at 1
    uint8_t     discrepancy_marker;
    uint8_t     b1;
    uint8_t     b2;

    if (done_flag)
    {
        done_flag = 0;
        return 0;
    }

    onewire_reset(&detect);
    if (!detect)
    {
        last_discrepancy = 0;
        return 0;
    }

    bit = 1;
    discrepancy_marker = 0;
    onewire_send_byte(OW_CMD_SEARCH_ROM);

    do
    {
        b1 = onewire_recv_bit();
        b2 = onewire_recv_bit();

        if (b1 == 1 && b2 == 1)
        {
            // shouldn't really get this
            last_discrepancy = 0;
            return 0;
        }
        else
        if (b1 == 0 && b2 == 0)
        {
            // clash between 2 or more devices
            if (bit == last_discrepancy)
            {
                serial |= mask;
            }
            else
            {
                if (bit > last_discrepancy)
                {
                    serial &= ~mask;
                    discrepancy_marker = bit;
                }
                else
                {
                    if (serial & mask)
                    {
                        discrepancy_marker = bit;
                    }
                }
            }
        }
        else
        {
            // no clash between devices
            if (b1)
                serial |= mask;
            else
                serial &= ~mask;
        }

        if (serial & mask)
            onewire_send_bit(1);
        else
            onewire_send_bit(0);
        
        bit++;
        mask <<= 1;

    } while (bit <= 64);

    last_discrepancy = discrepancy_marker;
    if (last_discrepancy == 0)
        done_flag = 1;

    *x = serial;

    return 1;
}

uint8_t
onewire_scan_first(uint64_t *serial)
{
    last_discrepancy = 0;
    done_flag = 0;

    return onewire_scan_next(serial);
}
#endif /* AVR_FEATURE_ONEWIRE_ENABLE_SEARCHROM */

#if AVR_FEATURE_ONEWIRE_ENABLE_MATCHROM
/* 
 * Match ROM
 */
void
onewire_cmd_match_rom(uint64_t rom)
{
    onewire_send_byte(OW_CMD_MATCH_ROM);

    for (uint8_t bit = 0; bit < 64; bit++)
    {
        onewire_send_bit((uint8_t)(rom & 0x1LL));
        rom >>= 1;
    }
}
#endif /* AVR_FEATURE_ONEWIRE_ENABLE_MATCHROM */
