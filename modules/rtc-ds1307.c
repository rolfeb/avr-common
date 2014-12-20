/*
 * Implement support for DS1307 RTC
 */
#include "avr-common.h"
#include "rtc-ds1307.h"
#include "i2c.h"

#define UNIX_EPOCH_OFFSET   2208988800UL
#define SECONDS_PER_DAY     86400UL
#define LEAP_YEAR(Y)        (((Y) % 4) == 0 && ( ((Y) % 100) != 0 || ((Y) % 400) == 0))
#define DAYS_IN_YEAR(Y)     (LEAP_YEAR(Y) ? 366 : 365)
#define DAYS_IN_MONTH(M, Y) \
            ((M) == 2 ? (LEAP_YEAR(Y) ? 29 : 28) : \
            (((M) == 4 || (M) == 6 || (M) == 9 || (M) == 11) ? 30 : \
            31))

/*
 * Defined oscillator bit
 */
#define R00_CH              7

/*
 * Define controller register bits
 */
#define R07_OUT             7
#define R07_SQWE            4
#define R07_RS1             1
#define R07_RS0             0

void
rtc_set_time_from_ntp(uint32_t ntp_time)
{
    /*
     * Convert to Unix epoch for quicker calculations
     */
    ntp_time -= UNIX_EPOCH_OFFSET;

    /*
     * Split into days and seconds, and then break down into datetime
     * components.
     */
    uint32_t daytime = ntp_time % SECONDS_PER_DAY;
    uint32_t days = (ntp_time - daytime) / SECONDS_PER_DAY;

    uint8_t hh, mm, ss, d, m, dow;
    uint16_t y;

    ss  = daytime % 60;
    daytime = (daytime - ss) / 60;
    mm = daytime % 60;
    hh = (daytime - mm) / 60;

    dow = (days + 3) % 7;

    y = 1970;
    while (days > DAYS_IN_YEAR(y))
    {
        days -= DAYS_IN_YEAR(y);
        y++;
    }
    m = 1;
    while (days > DAYS_IN_MONTH(m, y))
    {
        days -= DAYS_IN_MONTH(m, y);
        m++;
    }
    d = days + 1;   // 1-based

    /*
     * Set up RTC registers to write out
     */
    uint8_t reg[8];

    reg[0] = (ss / 10) << 4 | ss % 10;
    reg[1] = (mm / 10) << 4 | mm % 10;
    reg[2] = (hh / 10) << 4 | hh % 10;
    reg[3] = dow + 1;
    reg[4] = (d / 10) << 4 | d % 10;
    reg[5] = (m / 10) << 4 | m % 10;
    reg[6] = ((y - 2000) / 10) << 4 | (y - 2000) % 10;
    reg[7] = 0;

    /*
     * Start at address 0x00 and write out all 8 registers
     */
    if (i2c_start(I2C_ADDR_DS1307, I2C_SLA_W) != 0)
        goto ERROR;

    if (i2c_send_byte(0x00) != 0)
        goto ERROR;

    for (uint8_t i = 0; i < 8; i++)
    {
        if (i2c_send_byte(reg[i]) != 0)
            goto ERROR;
    }
    i2c_stop();

ERROR:
    return;
}

uint8_t
rtc_get_time(uint8_t *sec, uint8_t *min, uint8_t *hr, uint8_t *day, uint8_t *month, uint8_t *year, uint8_t *dow)
{
    uint8_t r;

    /*
     * Set the address pointer to 0x00 and read in the first 7 registers
     */
    if (i2c_start(I2C_ADDR_DS1307, I2C_SLA_W) != 0)
        goto ERROR;

    if (i2c_send_byte(0x00) != 0)
        goto ERROR;

    if (i2c_start(I2C_ADDR_DS1307, I2C_SLA_R) != 0)
        goto ERROR;

    r = i2c_read_byte_ack();
    *sec = ((r & 0x70) >> 4) * 10 + (r & 0x0f);

    r = i2c_read_byte_ack();
    *min = ((r & 0x70) >> 4) * 10 + (r & 0x0f);

    r = i2c_read_byte_ack();
    *hr = ((r & 0x30) >> 4) * 10 + (r & 0x0f);

    r = i2c_read_byte_ack();
    *dow = r & 0x07;

    r = i2c_read_byte_ack();
    *day = ((r & 0x30) >> 4) * 10 + (r & 0x0f);

    r = i2c_read_byte_ack();
    *month = ((r & 0x10) >> 4) * 10 + (r & 0x0f);

    r = i2c_read_byte_nack();
    *year = ((r & 0xf0) >> 4) * 10 + (r & 0x0f);

    i2c_stop();

    return 0;

ERROR:
    return 1;
}

uint8_t
rtc_init(void)
{
    /*
     * Set the address pointer to 0x00 and start the oscillator
     */
    if (i2c_start(I2C_ADDR_DS1307, I2C_SLA_W) != 0)
        goto ERROR;

    if (i2c_send_byte(0x00) != 0)
        goto ERROR;

    if (i2c_send_byte(R00_CH << 1) != 0)
        goto ERROR;

    i2c_stop();

    return 0;

ERROR:
    return 1;
}
