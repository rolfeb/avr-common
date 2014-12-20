/*
 * Routines to interfce with Modtronix LCD2S Serial LCD
 */
#include <stdint.h>

#include MCU_H
#include "i2c.h"
#include "lcd2s.h"

static uint8_t i2c_address;

static uint8_t
lcd_send_command(uint8_t cmd)
{
    uint8_t r;

    if ((r = i2c_start(i2c_address, I2C_SLA_W)) != 0)
        return r + 10;

    if ((r = i2c_send_byte(cmd)) != 0)
        return r + 20;

    i2c_stop();

    return 0;
}

static uint8_t
lcd_send_command_arg1(uint8_t cmd, uint8_t arg1)
{
    uint8_t r;

    if ((r = i2c_start(i2c_address, I2C_SLA_W)) != 0)
        return r + 10;

    if ((r = i2c_send_byte(cmd)) != 0)
        return r + 20;

    if ((r = i2c_send_byte(arg1)) != 0)
        return r + 30;

    i2c_stop();

    return 0;
}

static uint8_t
lcd_send_command_varg(uint8_t cmd, uint8_t nargs, uint8_t *vec)
{
    uint8_t r;

    if ((r = i2c_start(i2c_address, I2C_SLA_W)) != 0)
        return r + 10;

    if ((r = i2c_send_byte(cmd)) != 0)
        return r + 20;

    for (uint8_t i = 0; i < nargs; i++)
    {
        if ((r = i2c_send_byte(vec[i])) != 0)
            return r + 30 + i * 10;
    }

    i2c_stop();

    return 0;
}

void
lcd_init(uint8_t addr)
{
    i2c_address = addr;
}

uint8_t
lcd_set_backlight(uint8_t on)
{
    if (on)
        return lcd_send_command(LCD_CMD_SET_BACKLIGHT_ON);
    else
        return lcd_send_command(LCD_CMD_SET_BACKLIGHT_OFF);
}

uint8_t
lcd_set_backlight_level(uint8_t level)
{
    return lcd_send_command_arg1(LCD_CMD_SET_BACKLIGHT_LEVEL, level);
}

uint8_t
lcd_set_contrast(uint8_t level)
{
    return lcd_send_command_arg1(LCD_CMD_SET_CONTRAST, level);
}

uint8_t
lcd_goto_origin(void)
{
    return lcd_send_command(LCD_CMD_GOTO_ORIGIN);
}

uint8_t
lcd_goto_position(uint8_t row, uint8_t column)
{
    uint8_t vec[2];

    vec[0] = row;
    vec[1] = column;

    return lcd_send_command_varg(LCD_CMD_GOTO_POSITION, 2, vec);
}

uint8_t
lcd_clear_display(void)
{
    return lcd_send_command(LCD_CMD_CLEAR);
}

uint8_t
lcd_load_charset(uint8_t set)
{
    return lcd_send_command_arg1(LCD_CMD_LOAD_CHARSET, set);
}

uint8_t
lcd_write_parsed_string(const char *str)
{
    uint8_t r;

    if ((r = i2c_start(i2c_address, I2C_SLA_W)) != 0)
        return r + 10;

    if ((r = i2c_send_byte(LCD_CMD_WRITE_STRING)) != 0)
        return r + 20;

    for (const char *p = str; *p; p++)
    {
        if ((r = i2c_send_byte(*p)) != 0)
            return r + 30;
    }

    i2c_stop();

    return 0;
}

uint8_t
lcd_write_parsed_string_P(PGM_P str)
{
    uint8_t r;
    uint8_t b;

    if ((r = i2c_start(i2c_address, I2C_SLA_W)) != 0)
        return r + 10;

    if ((r = i2c_send_byte(LCD_CMD_WRITE_STRING)) != 0)
        return r + 20;

    for (;;)
    {
        if ((b = pgm_read_byte(str++)) == 0)
            break;

        if ((r = i2c_send_byte(b)) != 0)
            return r + 30;
    };

    i2c_stop();

    return 0;
}

uint8_t
lcd_write_large_num_string(const char *str)
{
    uint8_t r;

    if ((r = i2c_start(i2c_address, I2C_SLA_W)) != 0)
        return r + 10;

    if ((r = i2c_send_byte(LCD_CMD_WRITE_LARGE_NUM_STRING)) != 0)
        return r + 20;

    for (const char *p = str; *p; p++)
    {
        if ((r = i2c_send_byte(*p)) != 0)
            return r + 30;
    }

    i2c_stop();

    return 0;
}

uint8_t
lcd_get_status_byte(uint8_t *status)
{
    uint8_t r;

    if ((r = i2c_start(i2c_address, I2C_SLA_W)) != 0)
        return r + 10;

    if ((r = i2c_send_byte(LCD_CMD_GET_DEVICE_STATUS)) != 0)
        return r + 20;

    if ((r = i2c_rep_start(i2c_address, I2C_SLA_R)) != 0)
        return r + 30;

    uint8_t v = i2c_read_byte_nack();

    i2c_stop();

    *status = v;

    return 0;
}

uint8_t
lcd_scroll_display(uint8_t direction)
{
    switch (direction)
    {
    case LCD_SCROLL_UP:
        return lcd_send_command(LCD_CMD_SHIFT_UP);
    case LCD_SCROLL_DOWN:
        return lcd_send_command(LCD_CMD_SHIFT_DOWN);
    case LCD_SCROLL_LEFT:
        return lcd_send_command(LCD_CMD_SHIFT_LEFT);
    case LCD_SCROLL_RIGHT:
        return lcd_send_command(LCD_CMD_SHIFT_RIGHT);
    }

    return 1;
}

uint8_t
lcd_block_cursor(uint8_t enable)
{
    if (enable)
        return lcd_send_command(LCD_CMD_BLOCK_CURSOR_ON);
    else
        return lcd_send_command(LCD_CMD_BLOCK_CURSOR_OFF);
}

uint8_t
lcd_underline_cursor(uint8_t enable)
{
    if (enable)
        return lcd_send_command(LCD_CMD_UNDERLINE_CURSOR_ON);
    else
        return lcd_send_command(LCD_CMD_UNDERLINE_CURSOR_OFF);
}

uint8_t
lcd_load_character(uint8_t id, const uint8_t *data)
{
    uint8_t vec[9];

    vec[0] = id & 0x07;
    for (uint8_t i = 0; i < 8; i++)
        vec[i + 1] = data[i];

    return lcd_send_command_varg(LCD_CMD_LOAD_CHARACTER, 9, vec);
}
