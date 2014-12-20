#define PICASO_REPLY_ACK            0x06
#define PICASO_REPLY_NAK            0x15

#define PICASO_MAKE_COLOUR(R, G, B) \
    (((R) & 0x07) + (((G) & 0x07) << 3) + (((B) & 0x03) << 6))

#define PICASO_FONT_SIZE_5x7        0x00
#define PICASO_FONT_SIZE_8x8        0x01
#define PICASO_FONT_SIZE_8x12       0x02

#define PICASO_VIDEO_MODE_QVGA      0x00
#define PICASO_VIDEO_MODE_VGA       0x01
#define PICASO_VIDEO_MODE_SVGA      0x02

#define PICASO_PEN_MODE_FILL        0x00
#define PICASO_PEN_MODE_EDGE        0x01

extern void
picaso_init(void);

extern int8_t
picaso_autobaud(void);

extern int8_t
picaso_erase(void);

extern int8_t
picaso_set_display_page(uint8_t page);

extern int8_t
picaso_set_read_page(uint8_t page);

extern int8_t
picaso_set_write_page(uint8_t page);

extern int8_t
picaso_set_video_mode(uint8_t mode);

extern int8_t
picaso_set_background(uint8_t colour);

extern int8_t
picaso_set_pen_mode(uint8_t mode);

extern int8_t
picaso_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t colour);

extern int8_t
picaso_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t colour);

extern int8_t
picaso_draw_circle(uint16_t x, uint16_t y, uint16_t radius, uint8_t colour);

extern int8_t
picaso_draw_text_raw(uint16_t x, uint16_t y, uint8_t font, uint8_t colour, uint8_t width, uint8_t height, char *text);

extern int8_t
picaso_draw_text(uint8_t column, uint8_t row, uint8_t font, uint8_t colour, char *text);

extern int8_t
picaso_block_copy(uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t width, uint16_t height, uint8_t src_page, uint8_t dst_page);
