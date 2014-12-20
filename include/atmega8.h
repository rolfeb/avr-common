#ifndef __INCLUDE_ATMEGA8_H
#define __INCLUDE_ATMEGA8_H

#include <avr/io.h>
#include <avr/interrupt.h>


#define ENABLE_EXTERNAL_INT0()                                  \
    do {                                                        \
        /* generate interrupt on falling edge of INT0 */        \
        sbi(EICRA, ISC01);                                      \
        cbi(EICRA, ISC00);                                      \
                                                                \
        /* enable interrupts for INT0 */                        \
        sbi(EIMSK, INT0);                                       \
    } while (0)

/*
 * Define parameters for the I2C interface
 */
#define AVR_I2C_DDR         DDRC
#define AVR_I2C_PORT        PORTC
#define AVR_I2C_PORT_SCL    PC5
#define AVR_I2C_PORT_SDA    PC4

#ifndef AVR_I2C_CLOCK_HZ
# define AVR_I2C_CLOCK_HZ   100000L

/*
 * Define parameters for the SPI interface
 */
#define AVR_SPI_DDR             DDRB
#define AVR_SPI_PORT            PORTB
#define AVR_SPI_PORT_SS         PB2
#define AVR_SPI_PORT_MOSI       PB3
#define AVR_SPI_PORT_SCK        PB5

#define AVR_SPI_DATA_REGISTER   SPDR

#define spi_enable()            do { sbi(SPCR, SPE); } while(0)

#define spi_write_is_complete() (SPSR & (1<<SPIF))

#define spi_set_master_mode()   do { sbi(SPCR, MSTR); } while(0)

#define spi_set_speed_osc16()   do { sbi(SPCR, SPR0); } while(0)

#endif


/*
 * Define parameters for the default UART
 */
#define UBRR(CPU, BAUD) ((((((CPU) * 10) / (16L * (BAUD))) + 5) / 10) - 1)

#define uart0_init(baud)                                        \
    do {                                                        \
        UBRRH = (UBRR(F_CPU, baud) & 0xff00) >> 8;              \
        UBRRL = (UBRR(F_CPU, baud) & 0x00ff);                   \
                                                                \
        sbi(UCSRB, RXEN);                                       \
        sbi(UCSRB, TXEN);                                       \
                                                                \
        /* NOTE: must set URSEL to address UCSRC register */    \
        /* not the UBRRH register (they are shared) */          \
        UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);               \
                                                                \
        uart0_open_stdout();                                    \
    } while (0)

#define uart0_enable_rx_interrupts()                            \
    do {                                                        \
        sbi(UCSRB, RXCIE);                                      \
    } while (0)

#define AVR_UART0_DATA_REGISTER                     UDR

#define AVR_UART0_STATUS_REGISTER                   UCSRA

#define AVR_UART0_STATUS_REGISTER_DATA_READY_MASK   UDRE

#endif /* __INCLUDE_ATMEGA8_H */


