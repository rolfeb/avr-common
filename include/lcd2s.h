#ifndef __INCLUDE_LCD2S_H
#define __INCLUDE_LCD2S_H

#include <avr/pgmspace.h>

#define LCD_CMD_BLOCK_CURSOR_OFF        0x10
#define LCD_CMD_UNDERLINE_CURSOR_OFF    0x11

#define LCD_CMD_BLOCK_CURSOR_ON         0x18
#define LCD_CMD_UNDERLINE_CURSOR_ON     0x19

#define LCD_CMD_SET_BACKLIGHT_ON        0x28
#define LCD_CMD_SET_BACKLIGHT_OFF       0x20

#define LCD_CMD_WRITE_STRING            0x80
#define LCD_CMD_SET_BACKLIGHT_LEVEL     0x81
#define LCD_CMD_SET_CONTRAST            0x82

#define LCD_CMD_SHIFT_RIGHT             0x85
#define LCD_CMD_SHIFT_LEFT              0x86
#define LCD_CMD_SHIFT_UP                0x87
#define LCD_CMD_SHIFT_DOWN              0x88

#define LCD_CMD_GOTO_POSITION           0x8a
#define LCD_CMD_GOTO_ORIGIN             0x8b
#define LCD_CMD_CLEAR                   0x8c
#define LCD_CMD_LOAD_CHARSET            0x8e
#define LCD_CMD_WRITE_LARGE_NUM_STRING  0x8f

#define LCD_CMD_LOAD_CHARACTER          0x92

#define LCD_CMD_GET_DEVICE_STATUS       0xd0


/*
 * Default address for Modtronix LCD2S Serial LCD Display
 */
#define I2C_ADDR_LCD2S      0x28

/*
 * Codes for display scroll direction
 */
#define LCD_SCROLL_UP       0
#define LCD_SCROLL_DOWN     1
#define LCD_SCROLL_LEFT     2
#define LCD_SCROLL_RIGHT    3


extern void
lcd_init(uint8_t addr);

extern uint8_t
lcd_set_backlight(uint8_t on);

uint8_t
lcd_set_backlight_level(uint8_t level);

extern uint8_t
lcd_set_contrast(uint8_t level);

extern uint8_t
lcd_goto_origin(void);

extern uint8_t
lcd_goto_position(uint8_t row, uint8_t column);

extern uint8_t
lcd_clear_display(void);

extern uint8_t
lcd_load_charset(uint8_t set);

extern uint8_t
lcd_write_parsed_string(const char *str);

extern uint8_t
lcd_write_parsed_string_P(PGM_P str);

extern uint8_t
lcd_write_parsed_string_at(uint8_t row, uint8_t column, const char *str);

extern uint8_t
lcd_write_large_num_string(const char *str);

extern uint8_t
lcd_get_status_byte(uint8_t *status);

extern uint8_t
lcd_scroll_display(uint8_t direction);

extern uint8_t
lcd_block_cursor(uint8_t enable);

extern uint8_t
lcd_underline_cursor(uint8_t enable);

uint8_t
lcd_load_character(uint8_t id, const uint8_t *data);

#endif /* __INCLUDE_LCD2S_H */
