#ifndef __INCLUDE_UDP_H
#define __INCLUDE_UDP_H

#include <stdint.h>

/*
 * UDP header definitions
 */
#define UDP_HEADER_LEN              8

#define UDP_PORT_NTP                123

// source port
#define UDP_GET_SRC_PORT(UDP)       READ_2BYTE(UDP, 0)
#define UDP_SET_SRC_PORT(UDP, V)    WRITE_2BYTE(UDP, 0, V)

// destination port
#define UDP_GET_DST_PORT(UDP)       READ_2BYTE(UDP, 2)
#define UDP_SET_DST_PORT(UDP, V)    WRITE_2BYTE(UDP, 2, V)

// length
#define UDP_GET_LENGTH(UDP)         READ_2BYTE(UDP, 4)
#define UDP_SET_LENGTH(UDP, V)      WRITE_2BYTE(UDP, 4, V)

// checksum
#define UDP_GET_CKSUM(UDP)          READ_2BYTE(UDP, 6)
#define UDP_SET_CKSUM(UDP, V)       WRITE_2BYTE(UDP, 6, V)

#endif /* __INCLUDE_UDP_H */
