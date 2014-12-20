#ifndef __INCLUDE_ETH_H
#define __INCLUDE_ETH_H

#if 0
#define READ_2BYTE(PKT, OFFSET) \
    ( ((uint16_t)(PKT)[OFFSET] << 8) + (uint16_t)(PKT)[OFFSET+1] )

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
#endif

#define ETH_HEADER_LEN      14

#define ETH_DST_OFFSET      0
#define ETH_SRC_OFFSET      6
#define ETH_TYPE_OFFSET     12
#define ETH_DATA_OFFSET     14

#define ETH_TYPE(ETH)               READ_2BYTE(ETH, 12)
#define ETH_SET_TYPE(ETH, VALUE)    WRITE_2BYTE(ETH, 12, VALUE)


#define ETH_PROTOCOL_IP     0x0800
#define ETH_PROTOCOL_ARP    0x0806

/*
typedef void (eth_handler_t)(uint8_t *eth, uint8_t *pkt, uint16_t bytes);

extern void     eth_set_address(uint8_t *mac);

extern uint8_t  *eth_get_address(void);

extern void     eth_make_reply(uint8_t *eth);

extern void     eth_register_protocol_handler(uint16_t protocol, eth_handler_t *pkt_fn);

extern void     eth_process_packet(uint8_t *pkt, uint16_t bytes);
*/

#endif /* __INCLUDE_ETH_H */
