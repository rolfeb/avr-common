/*
 * Module for driving the default UART on an AVR
 */
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include MCU_H
#include "avr-common.h"
#include "uart0.h"

/*
 * The buffer containing text recived on the serial port
 */
#define COMMAND_BUF_LEN     32

static unsigned char        received_text[COMMAND_BUF_LEN+1];
static volatile uint8_t     next_ptr            = 0;
static volatile uint8_t     received_flag       = 0;

static int
push_char(char c, FILE *stream)
{
#if 0
    if (c == '\n')
        push_char('\r', stream);
#endif

    while (!uart0_tx_buffer_empty())
        continue;

    uart0_send_byte(c);

    while (!uart0_tx_send_complete())
        continue;

    return 0;
}

static int
get_char(FILE *stream)
{
    uint8_t data;

    if (!uart0_rx_data_available())
        return EOF;

    data = AVR_UART0_DATA_REGISTER;

    if (uart0_rx_error())
        return -2;

    return data;
}

void
uart0_open_stdout(void)
{
    static FILE serial  = FDEV_SETUP_STREAM(push_char, get_char,
                                _FDEV_SETUP_RW);
    uint8_t     dummy;

    stdout = &serial;
    stdin = &serial;

    // flush the receive buffer
    while ((AVR_UART0_STATUS_REGISTER
        & AVR_UART0_STATUS_REGISTER_DATA_RECVD_MASK) != 0)
        {
            dummy = AVR_UART0_DATA_REGISTER;
            (void)dummy;
        }
}

void
uart0_open_stdin(void)
{
    static FILE   serial  = FDEV_SETUP_STREAM(NULL, get_char,
                                _FDEV_SETUP_READ);

    stdin = &serial;
}

void
uart0_intr_handler(void)
{
    int     c   = AVR_UART0_DATA_REGISTER;

    if (received_flag)
        return;

    if (c == '\b' || c == '\177')
    {
        // handle backspace/delete
        if (next_ptr > 0)
        {
            push_char('\b', stdout);
            push_char(' ', stdout);
            push_char('\b', stdout);
            next_ptr--;
        }
    }
    else
    if (c == '\n' || c == '\r')
    {
        push_char('\n', stdout);
        received_text[next_ptr]  = '\0';
        next_ptr = 0;
        received_flag = 1;
    }
    else
    if (c != 0)
    {
        push_char(c, stdout);

        received_text[next_ptr++] = c;

        if (next_ptr == COMMAND_BUF_LEN)
        {
            push_char('\n', stdout);
            received_text[next_ptr]  = '\0';
            next_ptr = 0;
            received_flag = 1;
        }
    }
}

/*
 * Have we received a full line of text on the serial port?
 */
uint8_t
uart0_line_received(void)
{
    return received_flag;
}

/*
 * Return a pointer to a static buffer containing the received text
 */
unsigned char *
uart0_get_line(void)
{
    received_flag = 0;

    return received_text;
}
