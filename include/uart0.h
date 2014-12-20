#ifndef __INCLUDE_UART0_H
#define __INCLUDE_UART0_H

#include <stdint.h>

extern void
uart0_open_stdout(void);

extern void
uart0_open_stdin(void);

extern uint8_t        
uart0_line_received(void);

extern unsigned char *
uart0_get_line(void);

extern void           
uart0_intr_handler(void);

#endif /* __INCLUDE_UART0_H */
