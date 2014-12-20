/*
 * MOdule for driving the AVR SPI bus.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "avr-common.h"
#include MCU_H
#include "spi.h"

void
spi_start_tx(gpio_line_t *ss_pin)
{
    cbi(*ss_pin->p_out, ss_pin->line);
}

void
spi_end_tx(gpio_line_t *ss_pin)
{
    sbi(*ss_pin->p_out, ss_pin->line);
}

void
spi_send_byte(uint8_t c)
{
    AVR_SPI_DATA_REGISTER = c;

    while (!spi_write_is_complete())
        continue;
}

uint8_t
spi_send_recv_byte(uint8_t c)
{
    AVR_SPI_DATA_REGISTER = c;

    while(!spi_write_is_complete())
        continue;

    return AVR_SPI_DATA_REGISTER;
}

uint8_t
spi_receive_byte(void)
{
    return spi_send_recv_byte(0xff);
}

void
spi_init(void)
{
    /*
     * Put SPI pins in output mode. Must put SS in output/high mode, to
     * avoid being forced back into slave mode by low input.
     */
    sbi(AVR_SPI_DDR, AVR_SPI_PORT_SS);
    sbi(AVR_SPI_PORT, AVR_SPI_PORT_SS);

    sbi(AVR_SPI_DDR, AVR_SPI_PORT_MOSI);
    cbi(AVR_SPI_PORT, AVR_SPI_PORT_MOSI);

    sbi(AVR_SPI_DDR, AVR_SPI_PORT_SCK);
    cbi(AVR_SPI_PORT, AVR_SPI_PORT_SCK);

    /* set up the SPI module */
    spi_enable();
    spi_set_master_mode();
    spi_set_speed_osc16();
}

void
spi_init_slave(gpio_line_t *ss_pin)
{
    /* disable slave select for the moment (it is active low) */
    sbi(*ss_pin->ddr, ss_pin->line);
    sbi(*ss_pin->p_out, ss_pin->line);
}
