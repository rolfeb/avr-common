/*
 * Implement a 10ms clock counter
 */
#include <avr/io.h>

#include "avr-common.h"
#include "clock.h"

/*
 * 32-bit clock, with 10ms ticks
 */
static volatile uint32_t    tick_10ms;

inline void
clock_increment(void)
{
    tick_10ms++;
}

void
clock_init(void)
{
    // set clock source to external / 8
    cbi(TCCR1B, CS12);
    sbi(TCCR1B, CS11);
    cbi(TCCR1B, CS10);

    // set counter to CTC mode (count to OCR1A and reset)
    cbi(TCCR1B, WGM13);
    sbi(TCCR1B, WGM12);
    cbi(TCCR1A, WGM11);
    cbi(TCCR1A, WGM10);

    // set counter MAX to 7813
    // 6.25MHz / 8 / 7813 -> 99.994 Hz ~ 10.001 ms
    // OCR1A = 7813;
    OCR1A = F_CPU / 8L / 100L;

    // enable interrupts on timer match
    sbi(TIMSK1, OCIE1A);
    cbi(TIMSK1, TOIE1);

    tick_10ms = 0;

    // enable timer1
    cbi(PRR, PRTIM1);
}

inline uint32_t
clock_current_time(void)
{
    return tick_10ms;
}
