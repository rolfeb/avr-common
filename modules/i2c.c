/*
 * Module for driving the AVR I2C ("TWI") bus
 */
#include <avr/io.h>
#include <util/twi.h>
#include MCU_H
#include "avr-common.h"
#include "i2c.h"

static uint8_t  done_init;

void
i2c_init(void)
{
    if (!done_init)
    {
        sbi(AVR_I2C_DDR, AVR_I2C_PORT_SCL); // SCL pin in output mode
        sbi(AVR_I2C_DDR, AVR_I2C_PORT_SDA); // SDA pin in output mode

        cbi(AVR_I2C_PORT, AVR_I2C_PORT_SCL);
        cbi(AVR_I2C_PORT, AVR_I2C_PORT_SDA);

        // set prescaler to 1
        TWSR = 0;

        // set clock to 100kHz
        TWBR = ((F_CPU / (long)AVR_I2C_CLOCK_HZ) - 16) / 2;

        done_init = 1;
    }
}

void
i2c_slave_init(uint8_t address)
{
    if (!done_init)
    {
        sbi(AVR_I2C_DDR, AVR_I2C_PORT_SCL); // SCL pin in output mode
        sbi(AVR_I2C_DDR, AVR_I2C_PORT_SDA); // SDA pin in output mode

        cbi(AVR_I2C_PORT, AVR_I2C_PORT_SCL);
        cbi(AVR_I2C_PORT, AVR_I2C_PORT_SDA);

        // set prescaler to 1
        TWSR = 0;

        // set clock to 100kHz
        TWBR = ((F_CPU / (long)AVR_I2C_CLOCK_HZ) - 16) / 2;

        // set our address (ignore general call)
        TWAR = address << 1;

        // initialise I2C for listening
        TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWIE);

        done_init = 1;
    }
}

uint8_t
i2c_start(uint8_t address, uint8_t mode)
{
    uint8_t s;

    // send START condition
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

    // wait for transmission to complete
    while (!(TWCR & (1<<TWINT)))
        continue;

    // check for error
    s = TWSR & 0xf8;
    if (s != TW_START && s != TW_REP_START)
        return 1;

    // send device address
    TWDR = ((address & 0x7f) << 1) | mode;
    TWCR = (1<<TWINT) | (1<<TWEN);

    // wait for transmission to complete
    while (!(TWCR & (1<<TWINT)))
        continue;

    // check for error
    s = TWSR & 0xf8;
    if (s != TW_MT_SLA_ACK && s != TW_MR_SLA_ACK)
        return 2;

    return 0;
}

uint8_t
i2c_rep_start(uint8_t address, uint8_t mode)
{
    return i2c_start(address, mode);
}

uint8_t
i2c_send_byte(uint8_t data)
{
    // send data
    TWDR = data;
    TWCR = (1<<TWINT) | (1<<TWEN);

    // wait for transmission to complete
    while (!(TWCR & (1<<TWINT)))
        continue;

    // check for error
    if ((TWSR & 0xf8) != TW_MT_DATA_ACK)
        return 1;

    return 0;
}

uint8_t
i2c_send_byte_nowait(uint8_t data, uint8_t ack)
{
    // send data
    TWDR = data;

    if (ack)
        TWCR = (1<<TWIE) | (1<<TWEN) | (1<<TWEA);
    else
        TWCR = (1<<TWIE) | (1<<TWEN);

    return 0;
}

uint8_t
i2c_tx_busy(void)
{
    if (!(TWCR & (1<<TWINT)))
        return 1;

    return 0;
}

uint8_t
i2c_status(void)
{
    return TWSR & 0xf8;
}

uint8_t
i2c_read_byte_ack(void)
{
    // initialise receipt
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);

    // wait for receipt to complete
    while (!(TWCR & (1<<TWINT)))
        continue;

    return TWDR;
}

uint8_t
i2c_read_byte_nack(void)
{
    // initialise receipt
    TWCR = (1<<TWINT) | (1<<TWEN);

    // wait for receipt to complete
    while (!(TWCR & (1<<TWINT)))
        continue;

    return TWDR;
}

void
i2c_stop(void)
{
    // send STOP condition
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);

    // wait for transmission to complete and bus to be released
    while (!(TWCR & (1<<TWSTO)))
        continue;

    return;
}
