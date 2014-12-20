#ifndef __INCLUDE_TCP_H
#define __INCLUDE_TCP_H

#include <stdint.h>

/*
 * TCP header definitions
 */
#define TCP_HEADER_LEN          20

// source port
#define TCP_GET_SRC_PORT(TCP)       READ_2BYTE(TCP, 0)
#define TCP_SET_SRC_PORT(TCP, V)    WRITE_2BYTE(TCP, 0, V)

// destination port
#define TCP_GET_DST_PORT(TCP)       READ_2BYTE(TCP, 2)
#define TCP_SET_DST_PORT(TCP, V)    WRITE_2BYTE(TCP, 2, V)

#define TCP_PROTOCOL_HTTP       80

// sequence number
#define TCP_GET_SEQNO(TCP)          READ_4BYTE(TCP, 4)
#define TCP_SET_SEQNO(TCP, V)       WRITE_4BYTE(TCP, 4, V)

// acknowledgment number
#define TCP_GET_ACKNO(TCP)          READ_4BYTE(TCP, 8)
#define TCP_SET_ACKNO(TCP, V)       WRITE_4BYTE(TCP, 8, V)

// data offset
#define TCP_GET_DATA_OFFSET(TCP)    (((TCP)[12] & 0xf0) >> 4)
#define TCP_SET_DATA_OFFSET(TCP, V) \
    do { \
        (TCP)[12] = ((TCP)[12] & 0x0f) + ((V) << 4); \
    } while(0)

// flags
#define TCP_FLAG_FIN            (1<<0)
#define TCP_FLAG_SYN            (1<<1)
#define TCP_FLAG_RST            (1<<2)
#define TCP_FLAG_PSH            (1<<3)
#define TCP_FLAG_ACK            (1<<4)
#define TCP_FLAG_URG            (1<<5)

#define TCP_CLEAR_FLAGS(TCP)    do { (TCP)[13] &= 0xc0; } while(0)
#define TCP_SET_FLAG(TCP, FLAG) do { (TCP)[13] |= (FLAG); } while(0)
#define TCP_GET_FLAGS(TCP)      ((TCP)[13] & 0x3f)

// window
#define TCP_GET_WINDOW(TCP)     READ_2BYTE(TCP, 14)
#define TCP_SET_WINDOW(TCP, V)  WRITE_2BYTE(TCP, 14, V)

// checksum
#define TCP_GET_CKSUM(TCP)      READ_2BYTE(TCP, 16)
#define TCP_SET_CKSUM(TCP, V)   WRITE_2BYTE(TCP, 16, V)
            
// urgent pointer
#define TCP_GET_URG_PTR(TCP)    READ_2BYTE(TCP, 18)
#define TCP_SET_URG_PTR(TCP, V) WRITE_2BYTE(TCP, 18, V)

#if 0
typedef void (tcp_handler_t)(uint8_t *data, uint16_t *bytes);

extern void
tcp_expire_connections(void);

extern void
tcp_register_protocol_handler(uint16_t protocol, tcp_handler_t *pkt_fn);

extern void
tcp_make_reply(uint8_t *ip, uint8_t *tcp);

extern void
tcp_make_checksum(uint8_t *ip, uint8_t *tcp, uint16_t tcplen);

extern void
tcp_process_packet(uint8_t *eth, uint8_t *ip, uint8_t *data, unsigned int left);
#endif

#endif /* __INCLUDE_TCP_H */
