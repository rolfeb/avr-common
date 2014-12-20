#ifndef __INCLUDE_NETWORK_H
#define __INCLUDE_NETWORK_H

#define READ_BYTE(PKT, OFFSET) \
    ((PKT)[OFFSET])

#define READ_2BYTE(PKT, OFFSET) \
    ( ((uint16_t)(PKT)[OFFSET] << 8) + (uint16_t)(PKT)[OFFSET+1] )

#define WRITE_BYTE(PKT, OFFSET, VALUE) \
    do { \
        (PKT)[OFFSET] = (uint8_t)(VALUE); \
    } while(0)

#define WRITE_2BYTE(PKT, OFFSET, VALUE) \
    do { \
        (PKT)[OFFSET] = (uint8_t)(((VALUE) & 0xff00) >> 8); \
        (PKT)[OFFSET+1] = (uint8_t)((VALUE) & 0x00ff); \
    } while(0)

#define READ_4BYTE(PKT, OFFSET) \
    ( ((uint32_t)(PKT)[OFFSET] << 24) + ((uint32_t)(PKT)[OFFSET+1] << 16) + ((uint32_t)(PKT)[OFFSET+2] << 8) + (uint32_t)(PKT)[OFFSET+3] )

#define WRITE_4BYTE(PKT, OFFSET, VALUE) \
    do { \
        (PKT)[OFFSET] = (uint8_t)(((VALUE) & 0xff000000) >> 24); \
        (PKT)[OFFSET+1] = (uint8_t)(((VALUE) & 0x00ff0000) >> 16); \
        (PKT)[OFFSET+2] = (uint8_t)(((VALUE) & 0x0000ff00) >> 8); \
        (PKT)[OFFSET+3] = (uint8_t)((VALUE) & 0x000000ff); \
    } while(0)

#define IP_MATCH(IP1, IP2) \
    ((IP1)[0] == (IP2)[0] && (IP1)[1] == (IP2)[1] && (IP1)[2] == (IP2)[2] && (IP1)[3] == (IP2)[3])

#define IP_COPY(DST, SRC)  \
    do \
    { \
        for (uint8_t qq1 = 0; qq1 < 4; qq1++) \
            (DST)[qq1] = (SRC)[qq1]; \
    } while(0)

#define MAC_COPY(DST, SRC)  \
    do \
    { \
        for (uint8_t qq1 = 0; qq1 < 6; qq1++) \
            (DST)[qq1] = (SRC)[qq1]; \
    } while(0)

typedef void    ntp_reply_handler_t(uint32_t ntp_time);

typedef void    http_request_handler_t(uint8_t *data, uint16_t *reply_len);

extern void
network_set_http_request_handler(http_request_handler_t *f);

extern void
network_set_ntp_reply_handler(ntp_reply_handler_t *f);

extern void
network_read_packet(void);

extern void
tcp_expire_connections(void);

extern void
network_set_mac_address(uint8_t *ip);

extern uint8_t
*network_get_mac_address(void);

extern void
network_set_ip_address(uint8_t *ip);

extern uint8_t
*network_get_ip_address(void);

extern void
network_init(void);

extern uint8_t
network_send_ntp_request(uint8_t *server_ip);

#endif /* __INCLUDE_NETWORK_H */
