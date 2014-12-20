/*
 * Implement HP30 functions
 */
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#include "avr-common.h"
#include "i2c.h"
#include "hp30.h"

/*
 * Define I2C addresses for the HP30 barometer
 *
 * The documentation defines these as 0xa0 and 0xee, i.e. left shifted by 1
 * with the LSB being the R/W bit.
 */
#define HP30_ADDR_EEPROM    0x50
#define HP30_ADDR_SENSOR    0x77


struct hp30_coeff_t
{
    int16_t     c1;
    int16_t     c2;
    int16_t     c3;
    int16_t     c4;
    int16_t     c5;
    int16_t     c6;
    int16_t     c7;
    int8_t      a;
    int8_t      b;
    int8_t      c;
    int8_t      d;
};

/*
 * Calibration coefficients
 */
static struct hp30_coeff_t  hp30_c;

/*
 * XCLR (enable-high) pin
 */
static uint8_t  volatile    *xclr_port;
static uint8_t  volatile    *xclr_ddr;
static uint8_t              xclr_pin;

void
hp30_init(uint8_t volatile *port, uint8_t volatile *ddr, uint8_t pin)
{
    i2c_init();

    xclr_port = port;
    xclr_ddr = ddr;
    xclr_pin = pin;

    sbi(*xclr_ddr, xclr_pin);
    cbi(*xclr_port, xclr_pin);

    /*
     * Read calibration coefficients from the EEPROM
     */
    i2c_start(HP30_ADDR_EEPROM, I2C_SLA_W);
    i2c_send_byte(16);

    i2c_rep_start(HP30_ADDR_EEPROM, I2C_SLA_R);
    hp30_c.c1 = i2c_read_byte_ack() << 8;
    hp30_c.c1 |= i2c_read_byte_ack();
    hp30_c.c2 = i2c_read_byte_ack() << 8;
    hp30_c.c2 |= i2c_read_byte_ack();
    hp30_c.c3 = i2c_read_byte_ack() << 8;
    hp30_c.c3 |= i2c_read_byte_ack();
    hp30_c.c4 = i2c_read_byte_ack() << 8;
    hp30_c.c4 |= i2c_read_byte_ack();
    hp30_c.c5 = i2c_read_byte_ack() << 8;
    hp30_c.c5 |= i2c_read_byte_ack();
    hp30_c.c6 = i2c_read_byte_ack() << 8;
    hp30_c.c6 |= i2c_read_byte_ack();
    hp30_c.c7 = i2c_read_byte_ack() << 8;
    hp30_c.c7 |= i2c_read_byte_ack();
    hp30_c.a = i2c_read_byte_ack();
    hp30_c.b = i2c_read_byte_ack();
    hp30_c.c = i2c_read_byte_ack();
    hp30_c.d = i2c_read_byte_ack();
    i2c_stop();
}

void
hp30_read_values(hp30_measure_t *values)
{
    uint16_t    d1;
    uint16_t    d2;
    int32_t     t1;
    int32_t     dut;
    int32_t     off;
    int32_t     sens;
    int32_t     x;

    /*
     * Read temperature measurement
     */
    sbi(*xclr_port, xclr_pin);

    i2c_start(HP30_ADDR_SENSOR, I2C_SLA_W);
    i2c_send_byte(0xff);
    i2c_send_byte(0xe8);
    i2c_stop();

    /* wait 45ms */
    for (int i = 0; i < 9; i++)
        _delay_ms(5);

    i2c_start(HP30_ADDR_SENSOR, I2C_SLA_W);
    i2c_send_byte(0xfd);

    i2c_rep_start(HP30_ADDR_SENSOR, I2C_SLA_R);
    d2 = i2c_read_byte_ack() << 8;
    d2 |= i2c_read_byte_nack();
    i2c_stop();

    cbi(*xclr_port, xclr_pin);

    /*
     * Read pressure measurement
     */
    sbi(*xclr_port, xclr_pin);

    i2c_start(HP30_ADDR_SENSOR, I2C_SLA_W);
    i2c_send_byte(0xff);
    i2c_send_byte(0xf0);
    i2c_stop();

    /* wait 45ms */
    for (int i = 0; i < 9; i++)
        _delay_ms(5);

    i2c_start(HP30_ADDR_SENSOR, I2C_SLA_W);
    i2c_send_byte(0xfd);

    i2c_rep_start(HP30_ADDR_SENSOR, I2C_SLA_R);
    d1 = i2c_read_byte_ack() << 8;
    d1 |= i2c_read_byte_nack();
    i2c_stop();

    cbi(*xclr_port, xclr_pin);

    /*
     * Calculate actual values
     */

    t1 = ((int32_t)d2 - hp30_c.c5);
    t1 = (t1 * t1) >> 14;

    if ((int32_t)d2 >= hp30_c.c5)
        dut = (int32_t)d2 - hp30_c.c5 - ((t1 * hp30_c.a) >> hp30_c.c);
    else
        dut = (int32_t)d2 - hp30_c.c5 - ((t1 * hp30_c.b) >> hp30_c.c);

    off = (hp30_c.c2 + (((hp30_c.c4 - 1024) * dut) / 16384)) * 4;

    sens = hp30_c.c1 + ((hp30_c.c3 * dut) / 1024);

    x = ((sens * (d1 - 7168)) / 16384) - off;

    values->pressure = ((x * 10) / 32) + hp30_c.c7;

    values->temperature = 250 + ((dut * hp30_c.c6) / 65536) - (dut / (1 << hp30_c.d));
}
