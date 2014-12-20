#ifndef __INCLUDE_SPI_H
#define __INCLUDE_SPI_H

#include "avr-common.h"

extern void
spi_init(void);

extern void
spi_init_slave(gpio_line_t *ss_pin);

extern void
spi_start_tx(gpio_line_t *ss_pin);

extern void
spi_end_tx(gpio_line_t *ss_pin);

extern void
spi_send_byte(uint8_t c);

extern uint8_t
spi_send_recv_byte(uint8_t c);

extern uint8_t
spi_receive_byte(void);

#endif /* __INCLUDE_SPI_H */
