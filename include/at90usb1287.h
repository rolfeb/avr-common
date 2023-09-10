#ifndef __INCLUDE_AT90USB1287_H
#define __INCLUDE_AT90USB1287_H

#include <avr/io.h>
#include <avr/interrupt.h>

/*
 * Define parameters for the I2C interface
 */
#define AVR_I2C_DDR         DDRD
#define AVR_I2C_PORT        PORTD
#define AVR_I2C_PORT_SCL    PD0
#define AVR_I2C_PORT_SDA    PD1

#ifndef AVR_I2C_CLOCK_HZ
# define AVR_I2C_CLOCK_HZ   100000L
#endif

/*
 * Define parameters for the SPI interface
 */
#define AVR_SPI_DDR             DDRB
#define AVR_SPI_PORT            PORTB
#define AVR_SPI_PORT_SS         PB0
#define AVR_SPI_PORT_MOSI       PB2
#define AVR_SPI_PORT_MISO       PB3
#define AVR_SPI_PORT_SCK        PB1

#define AVR_SPI_DATA_REGISTER   SPDR

#define spi_enable()            do { sbi(SPCR, SPE); } while(0)

#define spi_write_is_complete() (SPSR & (1<<SPIF))

#define spi_set_master_mode()   do { sbi(SPCR, MSTR); } while(0)

#define spi_set_speed_osc16()   do { sbi(SPCR, SPR0); } while(0)

#if UNDEFINED 
/*
 * Define parameters for the default UART
 */
#define UBRR(CPU, BAUD) ((((((CPU) * 10) / (16L * (BAUD))) + 5) / 10) - 1)

#define uart0_init(baud)                                        \
    do {                                                        \
        UBRR0H = (UBRR(F_CPU, baud) & 0xff00) >> 8;             \
        UBRR0L = (UBRR(F_CPU, baud) & 0x00ff);                  \
                                                                \
        sbi(UCSR0B, RXEN0);                                     \
        sbi(UCSR0B, TXEN0);                                     \
                                                                \
        UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);                       \
                                                                \
        uart0_open_stdout();                                    \
    } while (0)

#define uart0_enable_rx_interrupts()                            \
    do {                                                        \
        sbi(UCSR0B, RXCIE0);                                    \
    } while (0)

#define AVR_UART0_DATA_REGISTER                     UDR0

#define AVR_UART0_STATUS_REGISTER                   UCSR0A

#define AVR_UART0_STATUS_REGISTER_DATA_READY_MASK   (1<<UDRE0)
#define AVR_UART0_STATUS_REGISTER_DATA_SENT_MASK    (1<<TXC0)
#define AVR_UART0_STATUS_REGISTER_DATA_RECVD_MASK   (1<<RXC0)

#define uart0_send_byte(C)          \
            do {                    \
                UCSR0A |= TXC0;     \
                UDR0 = (C);         \
            } while (0)

#define uart0_tx_buffer_empty()   ((UCSR0A & (1<<UDRE0)) != 0)
#define uart0_tx_send_complete()  ((UCSR0A & (1<<TXC0)) != 0)
#define uart0_rx_data_available() ((UCSR0A & (1<<RXC0)) != 0)

#define uart0_rx_error()    ((UCSR0A & ((1<<FE0)|(1<<DOR0)|(1<<UPE0))) != 0)
#endif /* UNDEFINED */

#endif /* __INCLUDE_AT90USB1287_H */


