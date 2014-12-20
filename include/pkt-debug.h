#ifndef __INCLUDE_PKT_DEBUG_H
#define __INCLUDE_PKT_DEBUG_H

extern void
pktdb_init(void);

extern void
pktdb_incr_recv(void);

extern void
pktdb_incr_sent(void);

extern void
pktdb_dump(char *buf);

extern void
pktdb_ether_type(uint16_t type);

#endif /* __INCLUDE_PKT_DEBUG_H */
