#ifndef __ETHERNET_ASSIGN_H__
#define __ETHERNET_ASSIGN_H__

/*
 * Our OUI
 */
#define ETHER_OUI       0x524342

/*
 * Allocated MAC addreses
 */
#define GOOGLE_CLIENT_ETHER_ID  0x000004
#define WLREC2_ETHER_ID         0x000005
#define UDP_ROUTER_ETHER_ID     0x000006

/*
 * Useful macros
 */
#define ETHER_MAC_0(ID) ((ETHER_OUI & 0xff0000) >> 16)
#define ETHER_MAC_1(ID) ((ETHER_OUI & 0x00ff00) >> 8)
#define ETHER_MAC_2(ID) ((ETHER_OUI & 0x0000ff))
#define ETHER_MAC_3(ID) (((ID)      & 0xff0000) >> 16)
#define ETHER_MAC_4(ID) (((ID)      & 0x00ff00) >> 8)
#define ETHER_MAC_5(ID) (((ID)      & 0x0000ff))

#endif /* __ETHERNET_ASSIGN_H__ */
