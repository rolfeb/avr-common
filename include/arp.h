#ifndef __INCLUDE_ARP_H
#define __INCLUDE_ARP_H

/*
 * ARP header definitions
 */
#define ARP_HEADER_LEN          28

#define ARP_HARDWARE_OFFSET     0
#define ARP_PROTOCOL_OFFSET     2
#define ARP_HWLEN_OFFSET        4
#define ARP_PRLEN_OFFSET        5
#define ARP_OPCODE_OFFSET       6
#define ARP_SRC_HARDWARE_OFFSET 8
#define ARP_SRC_PROTOCOL_OFFSET 14
#define ARP_DST_HARDWARE_OFFSET 18
#define ARP_DST_PROTOCOL_OFFSET 24
#define ARP_DATA_OFFSET         28

#define ARP_GET_HARDWARE(ARP)       READ_2BYTE(ARP, ARP_HARDWARE_OFFSET)
#define ARP_SET_HARDWARE(ARP, V)    WRITE_2BYTE(ARP, ARP_HARDWARE_OFFSET, V)
#define ARP_HARDWARE_ETHER          0x0001

#define ARP_GET_PROTOCOL(ARP)       READ_2BYTE(ARP, ARP_PROTOCOL_OFFSET)
#define ARP_SET_PROTOCOL(ARP, V)    WRITE_2BYTE(ARP, ARP_PROTOCOL_OFFSET, V)
#define ARP_PROTOCOL_IP             0x0800

#define ARP_GET_HWLEN(ARP)          READ_BYTE(ARP, ARP_HWLEN_OFFSET)
#define ARP_SET_HWLEN(ARP, V)       WRITE_BYTE(ARP, ARP_HWLEN_OFFSET, V)
#define ARP_HWLEN_ETHER             6

#define ARP_GET_PRLEN(ARP)          READ_BYTE(ARP, ARP_PRLEN_OFFSET)
#define ARP_SET_PRLEN(ARP, V)       WRITE_BYTE(ARP, ARP_PRLEN_OFFSET, V)
#define ARP_PRLEN_IP                4

#define ARP_GET_OPCODE(ARP)         READ_2BYTE(ARP, ARP_OPCODE_OFFSET)
#define ARP_SET_OPCODE(ARP, V)      WRITE_2BYTE(ARP, ARP_OPCODE_OFFSET, V)
#define ARP_OPCODE_REQUEST          0x0001
#define ARP_OPCODE_REPLY            0x0002

#define ARP_DST_IP_IS_US(arp, ip) ( \
    arp[ARP_DST_PROTOCOL_OFFSET+0] == ip[0] && \
    arp[ARP_DST_PROTOCOL_OFFSET+1] == ip[1] && \
    arp[ARP_DST_PROTOCOL_OFFSET+2] == ip[2] && \
    arp[ARP_DST_PROTOCOL_OFFSET+3] == ip[3] \
)

#define ARP_SET_SRC_PROTOCOL_ADDR_IP(ARP, IP) \
    do \
    { \
        (ARP)[ARP_SRC_PROTOCOL_OFFSET+0] = (IP)[0]; \
        (ARP)[ARP_SRC_PROTOCOL_OFFSET+1] = (IP)[1]; \
        (ARP)[ARP_SRC_PROTOCOL_OFFSET+2] = (IP)[2]; \
        (ARP)[ARP_SRC_PROTOCOL_OFFSET+3] = (IP)[3]; \
    } while(0)

#define ARP_SET_DST_PROTOCOL_ADDR_IP(ARP, IP) \
    do \
    { \
        (ARP)[ARP_DST_PROTOCOL_OFFSET+0] = (IP)[0]; \
        (ARP)[ARP_DST_PROTOCOL_OFFSET+1] = (IP)[1]; \
        (ARP)[ARP_DST_PROTOCOL_OFFSET+2] = (IP)[2]; \
        (ARP)[ARP_DST_PROTOCOL_OFFSET+3] = (IP)[3]; \
    } while(0)

#if 0
extern void     arp_process_packet(uint8_t *eth, uint8_t *pkt, uint16_t bytes);
#endif

#endif /* __INCLUDE_ARP_H */
