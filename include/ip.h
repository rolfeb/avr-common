#ifndef __INCLUDE_IP_H
#define __INCLUDE_IP_H

#define IP_HEADER_LEN           20

#define IP_VL_OFFSET            0
#define IP_TOS_OFFSET           1
#define IP_LENGTH_OFFSET        2
#define IP_ID_OFFSET            4
#define IP_FRAGMENT_OFFSET      6
#define IP_TTL_OFFSET           8
#define IP_PROTOCOL_OFFSET      9
#define IP_CKSUM_OFFSET         10
#define IP_SRC_IP_OFFSET        12
#define IP_DST_IP_OFFSET        16
#define IP_DATA_OFFSET          20

#define IP_VERSION(ip)          ((ip[IP_VL_OFFSET] & 0xf0) >> 4)

#define IP_GET_VERSION(IP)      (((IP)[IP_VL_OFFSET] & 0xf0) >> 4)
#define IP_SET_VERSION(IP, V) \
    do \
    { \
        (IP)[IP_VL_OFFSET] = ((IP)[IP_VL_OFFSET] & 0x0f) | ((V) << 4); \
    } while (0)

#define IP_VERSION_IPV4         4

#define IP_GET_HDR_LEN(IP)      ((IP)[IP_VL_OFFSET] & 0x0f)
#define IP_SET_HDR_LEN(IP, V) \
    do \
    { \
        (IP)[IP_VL_OFFSET] = ((IP)[IP_VL_OFFSET] & 0xf0) | (V); \
    } while (0)


#define IP_TOS(ip)              (ip[IP_TOS_OFFSET])
#define IP_GET_TOS(IP)          READ_BYTE((IP), IP_TOS_OFFSET)
#define IP_SET_TOS(IP, V)       WRITE_BYTE((IP), IP_TOS_OFFSET, (V))

#define IP_LENGTH_H(ip)         (ip[IP_LENGTH_OFFSET])
#define IP_LENGTH_L(ip)         (ip[IP_LENGTH_OFFSET+1])
#define IP_LENGTH(ip)           ((IP_LENGTH_H(ip) << 8) + IP_LENGTH_L(ip))
#define IP_GET_LENGTH(IP)       READ_2BYTE((IP), IP_LENGTH_OFFSET)
#define IP_SET_LENGTH(IP, V)    WRITE_2BYTE((IP), IP_LENGTH_OFFSET, (V))

#define IP_ID_H(ip)             (ip[IP_ID_OFFSET])
#define IP_ID_L(ip)             (ip[IP_ID_OFFSET+1])
#define IP_ID(ip)               ((IP_ID_H(ip) << 8) + IP_ID_L(ip))
#define IP_GET_ID(IP)           READ_2BYTE((IP), IP_ID_OFFSET)
#define IP_SET_ID(IP, V)        WRITE_2BYTE((IP), IP_ID_OFFSET, (V))

#define IP_FRAGMENT_H(ip)       (ip[IP_FRAGMENT_OFFSET])
#define IP_FRAGMENT_L(ip)       (ip[IP_FRAGMENT_OFFSET+1])
#define IP_FRAG_FLAGS(ip)       ((IP_FRAGMENT_H(ip) & 0xd0) >> 5)
#define IP_GET_FRAG_FLAGS(IP)   (((IP)[IP_FRAGMENT_OFFSET] & 0xd0) >> 5)
#define IP_SET_FRAG_FLAGS(IP, V) \
    do \
    { \
        (IP)[IP_FRAGMENT_OFFSET] = ((IP)[IP_FRAGMENT_OFFSET] & 0x1f) | ((V) << 5); \
    } while (0)

#define IP_FLAG_RESERVED        0x04
#define IP_FLAG_DONT_FRAGMENT   0x02
#define IP_FLAG_MORE_FRAGMENTS  0x01

#define IP_CLEAR_FLAGS(IP)      do { (IP)[IP_FRAGMENT_OFFSET] &= 0x1f; } while(0)
#define IP_SET_FLAG(IP, FLAG)   do { (IP)[IP_FRAGMENT_OFFSET] |= ((FLAG) << 5); } while(0)
#define IP_GET_FLAGS(IP)        (((IP)[IP_FRAGMENT_OFFSET] & 0xe0) >> 5)

#define IP_FRAG_OFFSET(ip)      (((IP_FRAGMENT_H(ip) & 0x1) << 8) + IP_FRAGMENT_L(ip))
#define IP_GET_FRAG_OFFSET(IP)  (READ_2BYTE((IP), IP_FRAGMENT_OFFSET) & 0x1fff)
#define IP_SET_FRAG_OFFSET(IP, V)   WRITE_2BYTE((IP), IP_FRAGMENT_OFFSET, (V) & 0x1fff)

#define IP_TTL(ip)              (ip[IP_TTL_OFFSET])
#define IP_GET_TTL(IP)          READ_BYTE((IP), IP_TTL_OFFSET)
#define IP_SET_TTL(IP, V)       WRITE_BYTE((IP), IP_TTL_OFFSET, (V))

#define IP_PROTOCOL(ip)         (ip[IP_PROTOCOL_OFFSET])
#define IP_GET_PROTOCOL(IP)     READ_BYTE((IP), IP_PROTOCOL_OFFSET)
#define IP_SET_PROTOCOL(IP, V)  WRITE_BYTE((IP), IP_PROTOCOL_OFFSET, (V))
#define IP_PROTOCOL_ICMP    1
#define IP_PROTOCOL_TCP     6
#define IP_PROTOCOL_UDP     17

#define IP_CKSUM_H(ip)          (ip[IP_CKSUM_OFFSET])
#define IP_CKSUM_L(ip)          (ip[IP_CKSUM_OFFSET+1])
#define IP_CKSUM(ip)            ((IP_CKSUM_H(ip) << 8) + IP_CKSUM_L(ip))
#define IP_GET_CKSUM(IP)        READ_2BYTE((IP), IP_CKSUM_OFFSET)
#define IP_SET_CKSUM(IP, V)     WRITE_2BYTE((IP), IP_CKSUM_OFFSET, (V))


#if 0
typedef void (ip_handler_t)(uint8_t *eth, uint8_t *ip, uint8_t *pkt, uint16_t bytes);

void        ip_set_address(uint8_t *ipaddr);

uint8_t     *ip_get_address(void);

void        ip_make_reply(uint8_t *ip);

void        ip_make_checksum(uint8_t *ip);

void        ip_register_protocol_handler(uint16_t protocol, ip_handler_t *pkt_fn);

void        ip_process_packet(uint8_t *eth, uint8_t *data, unsigned int left);
#endif

#endif /* __INCLUDE_IP_H */
