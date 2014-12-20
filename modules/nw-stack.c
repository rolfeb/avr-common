#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>

#include "avr-common.h"
#include "clock.h"
#include "task.h"
#include "enc28j60.h"

#include "nw-stack.h"

#ifndef AVR_FEATURE_NWSTACK_PKT_BUFFER_SIZE
#define AVR_FEATURE_NWSTACK_PKT_BUFFER_SIZE         256
#endif

#ifndef AVR_FEATURE_NWSTACK_MAX_TCP_CONNECTIONS
#define AVR_FEATURE_NWSTACK_MAX_TCP_CONNECTIONS     4
#endif

#ifndef AVR_FEATURE_NWSTACK_ARP_CACHE_SIZE
#define AVR_FEATURE_NWSTACK_ARP_CACHE_SIZE          3
#endif

/*
 * The packet buffer
 */
static uint8_t                  pkt[AVR_FEATURE_NWSTACK_PKT_BUFFER_SIZE];

/*
 * Callback functions
 */
static ntp_reply_handler_t      *ntp_reply_handler;
static http_request_handler_t   *http_request_handler;

/*
 * Forward declarations for protocol handlers
 */
static void
eth_process_packet(uint8_t *pkt, uint16_t bytes);

static void
arp_process_packet(uint8_t *eth, uint8_t *data, unsigned int left);

static void
ip_process_packet(uint8_t *eth, uint8_t *data, unsigned int left);

static void
icmp_process_packet(uint8_t *eth, uint8_t *ip, uint8_t *data, unsigned int left);

static void
tcp_process_packet(uint8_t *eth, uint8_t *ip, uint8_t *data, unsigned int left);

static void
udp_process_packet(uint8_t *eth, uint8_t *ip, uint8_t *data, unsigned int left);

static void
ntp_process_packet(uint8_t *eth, uint8_t *ip, uint8_t *udp, uint8_t *data, unsigned int left);

/****************************************************************************/
/* ETHERNET Layer */
/****************************************************************************/

#include "eth.h"

static uint8_t              mac_address[6];

static void
eth_set_address(uint8_t *mac)
{
    for (int i = 0; i < 6; i++)
        mac_address[i] = mac[i];
}

/*
 * Change ethernet packet into a reply
 */
static void
eth_make_reply(uint8_t *eth)
{
    /*
     * Rewrite src & dst MAC addresses
     */
    for (int i = 0; i < 6; i++)
    {
        eth[ETH_DST_OFFSET+i] = eth[ETH_SRC_OFFSET+i];
        eth[ETH_SRC_OFFSET+i] = mac_address[i];
    }
}

static void
eth_process_packet(uint8_t *pkt, uint16_t bytes)
{
    if (bytes < ETH_HEADER_LEN)
        return;

    uint8_t     *eth        = pkt;
    unsigned int consumed   = ETH_HEADER_LEN;

    /*
     * Hand-off IP and ARP to higher layers and ignore the rest
     */
    if (ETH_TYPE(eth) == ETH_PROTOCOL_IP)
        ip_process_packet(eth, pkt + consumed, bytes - consumed);
    else
    if (ETH_TYPE(eth) == ETH_PROTOCOL_ARP)
        arp_process_packet(eth, pkt + consumed, bytes - consumed);
}

uint8_t *
network_get_mac_address(void)
{
    return mac_address;
}

/****************************************************************************/
/* IP Layer */
/****************************************************************************/

#include "ip.h"

static uint8_t              ip_address[4];

static void
ip_set_address(uint8_t *ipaddr)
{
    for (int i = 0; i < 4; i++)
        ip_address[i] = ipaddr[i];
}

/*
 * Change IP packet into a reply
 */
static void
ip_make_reply(uint8_t *ip)
{
    /*
     * Rewrite src & dst IP addresses
     */
    for (int i = 0; i < 4; i++)
    {
        ip[IP_DST_IP_OFFSET+i] = ip[IP_SRC_IP_OFFSET+i];
        ip[IP_SRC_IP_OFFSET+i] = ip_address[i];
    }

    IP_SET_TTL(ip, 64);

    /*
     * We don't support fragmentation
     */
    IP_SET_FRAG_FLAGS(ip, IP_FLAG_DONT_FRAGMENT);
    IP_SET_FRAG_OFFSET(ip, 0);
}

/* 
 * Calculate the IP checksum and set in the packet
 */
static void
ip_make_checksum(uint8_t *ip)
{
    // calculate IP checksum
    IP_SET_CKSUM(ip, 0);

    uint32_t cksum = 0;
    uint8_t *ptr   = ip;

    uint8_t hdr_bytes   = IP_GET_HDR_LEN(ip) * 4;

    for (int i = 0; i < hdr_bytes; i += 2)
        cksum += ((uint32_t)(ptr[i]) << 8) + ptr[i+1];

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);

    cksum = (uint16_t)~cksum;

    IP_SET_CKSUM(ip, cksum);
}

/*
 * Process a received IP packet
 */
static void
ip_process_packet(uint8_t *eth, uint8_t *data, unsigned int left)
{
    if (left < IP_HEADER_LEN)
        return;

    uint8_t         *ip         = data;
    unsigned int    options_len = (IP_GET_HDR_LEN(ip) - 5) * 4;
    unsigned int    consumed    = IP_HEADER_LEN + options_len;

    left = IP_GET_LENGTH(ip) - consumed;

    /*
     * Hand-off ICMP, TCP, UDP to higher layers and ignore the rest
     */
    if (IP_PROTOCOL(ip) == IP_PROTOCOL_ICMP)
        icmp_process_packet(eth, ip, data + consumed, left);
    else
    if (IP_PROTOCOL(ip) == IP_PROTOCOL_TCP)
        tcp_process_packet(eth, ip, data + consumed, left);
    else
    if (IP_PROTOCOL(ip) == IP_PROTOCOL_UDP)
        udp_process_packet(eth, ip, data + consumed, left);
}

uint8_t *
network_get_ip_address(void)
{
    return ip_address;
}

/****************************************************************************/
/* ARP Layer */
/****************************************************************************/

#include "arp.h"

/*
 * ARP cache entry definition. Each entry takes 15 bytes
 */
typedef struct
{
    uint32_t    last_access;

    uint8_t     mac_address[6];
    uint8_t     ip_address[4];

    unsigned    alloc   :1;
    unsigned    valid   :1;
}
    arp_cache_entry_t;

/*
 * The ARP cache
 */
static arp_cache_entry_t    arp_cache[AVR_FEATURE_NWSTACK_ARP_CACHE_SIZE];

static void
arp_cache_init(void)
{
    for (uint8_t i = 0; i < AVR_FEATURE_NWSTACK_ARP_CACHE_SIZE; i++)
        arp_cache[i].alloc = arp_cache[i].valid = 0;
}

/*
 * Given an IP, address look for the corresponding MAC address in the ARP
 * cache.  Returns 0 if no match.
 */
static arp_cache_entry_t *
arp_cache_lookup(uint8_t *ip)
{
    for (uint8_t i = 0; i < AVR_FEATURE_NWSTACK_ARP_CACHE_SIZE; i++)
    {
        if
        (
            arp_cache[i].valid
            &&
            arp_cache[i].ip_address[0] == ip[0]
            &&
            arp_cache[i].ip_address[1] == ip[1]
            &&
            arp_cache[i].ip_address[2] == ip[2]
            &&
            arp_cache[i].ip_address[3] == ip[3]
        )
        {
            arp_cache[i].last_access = clock_current_time();
            return &arp_cache[i];
        }
    }

    return 0;   /* no entry in the ARP cache */
}

/*
 * Get a free cache slot, re-using the LRU entry if necessary
 */
static int
arp_get_cache_slot(void)
{
    uint8_t     oldest          = 255;
    uint32_t    oldest_access   = 0L;

    for (uint8_t i = 0; i < AVR_FEATURE_NWSTACK_ARP_CACHE_SIZE; i++)
    {
        if (!arp_cache[i].alloc)
            return i;

        if (oldest == 255 || arp_cache[i].last_access < oldest_access)
        {
            oldest = i;
            oldest_access = arp_cache[i].last_access;
        }
    }

    arp_cache[oldest].alloc = 0;
    arp_cache[oldest].valid = 0;
    arp_cache[oldest].last_access = clock_current_time();

    return oldest;
}

#if AVR_FEATURE_NWSTACK_PROCESS_ARP_REQUESTS
static void
handle_arp_request(uint8_t *eth, uint8_t *arp, unsigned int consumed)
{
    if (ARP_DST_IP_IS_US(arp, ip_address))
    {
        /*
         * This is an ARP request for our IP
         */
        eth_make_reply(eth);

        /*
         * Adjust ARP packet
         */
        ARP_SET_OPCODE(arp, ARP_OPCODE_REPLY);

        for (int i = 0; i < 6; i++)
        {
            arp[ARP_DST_HARDWARE_OFFSET+i] = arp[ARP_SRC_HARDWARE_OFFSET+i];
            arp[ARP_SRC_HARDWARE_OFFSET+i] = mac_address[i];
        }
        for (int i = 0; i < 4; i++)
        {
            arp[ARP_DST_PROTOCOL_OFFSET+i] = arp[ARP_SRC_PROTOCOL_OFFSET+i];
            arp[ARP_SRC_PROTOCOL_OFFSET+i] = ip_address[i];
        }

        // send reply
        enc28j60_send_packet(eth, (arp - eth) + consumed);
    }
}
#endif /* AVR_FEATURE_NWSTACK_PROCESS_ARP_REQUESTS */

static void
handle_arp_reply(uint8_t *eth, uint8_t *arp, unsigned int consumed)
{
    if (ARP_DST_IP_IS_US(arp, ip_address))
    {
        /*
         * If the ARP sender IP is an incomplete entry in the ARP cache, then
         * update it with the Ethernet address.
         */
        for (uint8_t i = 0; i < AVR_FEATURE_NWSTACK_ARP_CACHE_SIZE; i++)
        {
            if (!arp_cache[i].alloc || arp_cache[i].valid)
                continue;

            uint8_t *reply_ip   = &arp[ARP_SRC_PROTOCOL_OFFSET];
            uint8_t *reply_mac  = &arp[ARP_SRC_HARDWARE_OFFSET];

            if
            (
                reply_ip[0] == arp_cache[i].ip_address[0]
                &&
                reply_ip[1] == arp_cache[i].ip_address[1]
                &&
                reply_ip[2] == arp_cache[i].ip_address[2]
                &&
                reply_ip[3] == arp_cache[i].ip_address[3]
            )
            {
                for (int j = 0; j < 6; j++)
                    arp_cache[i].mac_address[j] = reply_mac[j];

                arp_cache[i].valid = 1;

                break;
            }
        }
    }
}

static void
arp_process_packet(uint8_t *eth, uint8_t *data, unsigned int left)
{
    if (left < ARP_HEADER_LEN)
        return;

    uint8_t         *arp       = data;
    unsigned int    consumed   = ARP_HEADER_LEN;

    if (ARP_GET_HARDWARE(arp) == ARP_HARDWARE_ETHER && ARP_GET_PROTOCOL(arp) == ARP_PROTOCOL_IP)
    {
        // IP over Ethernet

#if AVR_FEATURE_NWSTACK_PROCESS_ARP_REQUESTS
        if (ARP_GET_OPCODE(arp) == ARP_OPCODE_REQUEST)
            handle_arp_request(eth, arp, consumed);
        else
#endif /* AVR_FEATURE_NWSTACK_PROCESS_ARP_REQUESTS */
        if (ARP_GET_OPCODE(arp) == ARP_OPCODE_REPLY)
            handle_arp_reply(eth, arp, consumed);
    }

    return;
}

/*
 * Broadcast an ARP request for the given IP address.
 */
void
send_arp_request(uint8_t *req_ip_address)
{
    uint8_t *eth    = pkt;
    uint8_t *arp    = pkt + ETH_HEADER_LEN;
    int     len;

    cli();  /* disable reply handling to prevent buffer overwrite */

    /*
     * Create the ethernet header
     */
    for (uint8_t i = 0; i < 6; i++)
    {
        eth[ETH_SRC_OFFSET+i] = mac_address[i];
        eth[ETH_DST_OFFSET+i] = 0xff;

        arp[ARP_SRC_HARDWARE_OFFSET+i] = mac_address[i];
        arp[ARP_DST_HARDWARE_OFFSET+i] = 0;
    }
    ETH_SET_TYPE(eth, ETH_PROTOCOL_ARP);

    /*
     * Create the ARP header
     */
    ARP_SET_HARDWARE(arp, ARP_HARDWARE_ETHER);
    ARP_SET_PROTOCOL(arp, ARP_PROTOCOL_IP);
    ARP_SET_HWLEN(arp, ARP_HWLEN_ETHER);
    ARP_SET_PRLEN(arp, ARP_PRLEN_IP);

    ARP_SET_OPCODE(arp, ARP_OPCODE_REQUEST);

    ARP_SET_SRC_PROTOCOL_ADDR_IP(arp, ip_address);
    ARP_SET_DST_PROTOCOL_ADDR_IP(arp, req_ip_address);

    /*
     * Send the packet!
     */
    len = ETH_HEADER_LEN + ARP_HEADER_LEN;
    enc28j60_send_packet(eth, len);

    /*
     * Add a pending cache entry
     */
    uint8_t slot    = arp_get_cache_slot();

    for (uint8_t i = 0; i < 4; i++)
        arp_cache[slot].ip_address[i] = req_ip_address[i];

    arp_cache[slot].alloc = 1;
     
    sei();
}

/****************************************************************************/
/* ICMP Layer */
/****************************************************************************/

#include "icmp.h"

#if AVR_FEATURE_NWSTACK_PROCESS_PING_REQUESTS
static void
handle_echo_request(uint8_t *eth, uint8_t *ip, uint8_t *icmp, unsigned int left)
{
    if (ip[IP_DST_IP_OFFSET+0] == ip_address[0] &&
        ip[IP_DST_IP_OFFSET+1] == ip_address[1] &&
        ip[IP_DST_IP_OFFSET+2] == ip_address[2] &&
        ip[IP_DST_IP_OFFSET+3] == ip_address[3])
    {
        // this is our host IP, send a reply

        eth_make_reply(eth);
        ip_make_reply(ip);
        ip_make_checksum(ip);

        // adjust ICMP packet
        ICMP_SET_TYPE(icmp, ICMP_TYPE_ECHO_REPLY);

        // adjust ICMP checksum
        uint8_t delta   = (ICMP_TYPE_ECHO_REQUEST - ICMP_TYPE_ECHO_REPLY);
        if (ICMP_CKSUM_H(icmp) > (0xff-delta))
            ICMP_CKSUM_L(icmp)++;
        ICMP_CKSUM_H(icmp) += delta;

        // send reply
        enc28j60_send_packet(eth, (icmp - eth) + left);
    }
}
#endif /* AVR_FEATURE_NWSTACK_PROCESS_PING_REQUESTS */

static void
icmp_process_packet(uint8_t *eth, uint8_t *ip, uint8_t *data, unsigned int left)
{
    if (left < ICMP_HEADER_LEN)
        return;

    uint8_t         *icmp      = data;

#if AVR_FEATURE_NWSTACK_PROCESS_PING_REQUESTS
    if (ICMP_GET_TYPE(icmp) == ICMP_TYPE_ECHO_REQUEST)
        handle_echo_request(eth, ip, icmp, left);
#endif /* AVR_FEATURE_NWSTACK_PROCESS_PING_REQUESTS */
}

/****************************************************************************/
/* UDP Layer */
/****************************************************************************/

#include "udp.h"

static void
udp_process_packet(uint8_t *eth, uint8_t *ip, uint8_t *data, unsigned int left)
{
    if (left < UDP_HEADER_LEN)
        return;

    uint8_t         *udp        = data;
    unsigned int    consumed    = UDP_HEADER_LEN;

    // skip the UDP header
    data += consumed;
    left -= consumed;

    if (UDP_GET_DST_PORT(udp) == UDP_PORT_NTP)
        ntp_process_packet(eth, ip, udp, data, left);

    return;
}

/****************************************************************************/
/* TCP Layer */
/****************************************************************************/

#include "tcp.h"

#define TCP_STATE_LISTEN        0x00
#define TCP_STATE_CLOSED        0x01
#define TCP_STATE_SYNRCVD       0x02
#define TCP_STATE_SYNSENT       0x04
#define TCP_STATE_ESTABLISHED   0x05
#define TCP_STATE_FINWAIT1      0x06
#define TCP_STATE_FINWAIT2      0x07
#define TCP_STATE_CLOSING       0x08
#define TCP_STATE_TIMEWAIT      0x09
#define TCP_STATE_CLOSEWAIT     0x0a
#define TCP_STATE_LASTACK       0x0b

typedef struct {
    uint16_t        local_port;
    uint8_t         remote_ip[4];
    uint16_t        remote_port;
    uint8_t         state;
    uint32_t        seqno;
    unsigned long   msl2;
} tcb_t;

/*
 * TCP Control Block.
 * Entries are initialised with .state == 0 == TCP_STATE_LISTEN by default.
 */
static tcb_t    tcb[AVR_FEATURE_NWSTACK_MAX_TCP_CONNECTIONS];

void
tcp_expire_connections(void)
{
    uint32_t    clock   = clock_current_time();

    // close any TCP sockets in TIME_WAIT state
    for (int n = 0; n < AVR_FEATURE_NWSTACK_MAX_TCP_CONNECTIONS; n++)
    {
        // XXX: need to cater for wraparound!
        if (tcb[n].state == TCP_STATE_TIMEWAIT && clock >= tcb[n].msl2)
        {
            tcb[n].state = TCP_STATE_LISTEN;
        }
    }
}

static void
tcp_make_reply(uint8_t *ip, uint8_t *tcp)
{
    uint16_t tmp;

    tmp = TCP_GET_SRC_PORT(tcp);
    TCP_SET_SRC_PORT(tcp, TCP_GET_DST_PORT(tcp));
    TCP_SET_DST_PORT(tcp, tmp);
}

static void
tcpudp_make_checksum(uint8_t *ip, uint8_t *tcpudp, uint16_t pktlen, uint8_t is_tcp)
{
    if (is_tcp)
        TCP_SET_CKSUM(tcpudp, 0);
    else
        UDP_SET_CKSUM(tcpudp, 0);

    uint32_t cksum = 0;

    // pseudo IP header
    for (int i = 0; i < 8; i += 2)
        cksum += ((uint32_t)(ip[IP_SRC_IP_OFFSET+i]) << 8)  + ip[IP_SRC_IP_OFFSET+i+1];
    cksum += 0                                              + (uint32_t)IP_PROTOCOL(ip);
    cksum += (uint32_t)pktlen;

    // TCP/UDP header
    int i;
    for (i = 0; i < pktlen; i += 2)
        cksum += ((uint32_t)tcpudp[i] << 8) + tcpudp[i+1];

    if (i < pktlen - 1)
        cksum += (uint32_t)tcpudp[i+1] << 8;

    // handle 16-bit ones-complement overflow
    while (cksum >> 16)
        cksum = (cksum & 0xffff) + (cksum >> 16);

    if (is_tcp)
        TCP_SET_CKSUM(tcpudp, ~cksum & 0xffff);
    else
        UDP_SET_CKSUM(tcpudp, ~cksum & 0xffff);
}

static void
send_reply(uint8_t *eth, uint8_t *ip, uint8_t *tcp, uint8_t flags, uint32_t ackno,
    uint32_t seqno, uint16_t left)
{
    eth_make_reply(eth);
    ip_make_reply(ip);

    IP_SET_LENGTH(ip, IP_HEADER_LEN + TCP_HEADER_LEN + left);

    tcp_make_reply(ip, tcp);

    TCP_CLEAR_FLAGS(tcp);
    TCP_SET_FLAG(tcp, flags);

    TCP_SET_ACKNO(tcp, ackno);
    TCP_SET_SEQNO(tcp, seqno);

    TCP_SET_DATA_OFFSET(tcp, 5);    // no options

    ip_make_checksum(ip);
    tcpudp_make_checksum(ip, tcp, TCP_HEADER_LEN + left, 1);

    // send reply
    enc28j60_send_packet(eth, (tcp - eth) + TCP_HEADER_LEN + left);
}

static void
tcp_process_packet(uint8_t *eth, uint8_t *ip, uint8_t *data, unsigned int left)
{
    if (left < TCP_HEADER_LEN)
        return;

    uint8_t         *tcp        = data;
    unsigned int    options_len = (TCP_GET_DATA_OFFSET(tcp) * 4) - TCP_HEADER_LEN;
    unsigned int    consumed    = TCP_HEADER_LEN + options_len;

    // skip the TCP header
    data += consumed;
    left -= consumed;

    if (TCP_GET_DST_PORT(tcp) == TCP_PROTOCOL_HTTP)
    {
        int8_t unused  = -1;
        int8_t n;

        for (n = 0; n < AVR_FEATURE_NWSTACK_MAX_TCP_CONNECTIONS; n++)
        {
            if (tcb[n].state == TCP_STATE_LISTEN)
                unused = n;
            else
            if (tcb[n].local_port == TCP_GET_DST_PORT(tcp)
                && tcb[n].remote_port == TCP_GET_SRC_PORT(tcp)
                && tcb[n].remote_ip[0] == ip[IP_SRC_IP_OFFSET+0]
                && tcb[n].remote_ip[1] == ip[IP_SRC_IP_OFFSET+1]
                && tcb[n].remote_ip[2] == ip[IP_SRC_IP_OFFSET+2]
                && tcb[n].remote_ip[3] == ip[IP_SRC_IP_OFFSET+3])
            {
                // we have a connection already (in some state)
                break;
            }
        }
        if (n >= AVR_FEATURE_NWSTACK_MAX_TCP_CONNECTIONS)
        {
            if ((n = unused) == -1)
                goto FAIL;

            tcb[n].local_port = TCP_GET_DST_PORT(tcp);
            tcb[n].remote_port = TCP_GET_SRC_PORT(tcp);
            tcb[n].remote_ip[0] = ip[IP_SRC_IP_OFFSET+0];
            tcb[n].remote_ip[1] = ip[IP_SRC_IP_OFFSET+1];
            tcb[n].remote_ip[2] = ip[IP_SRC_IP_OFFSET+2];
            tcb[n].remote_ip[3] = ip[IP_SRC_IP_OFFSET+3];
            tcb[n].seqno = clock_current_time();
            tcb[n].msl2 = 0;
        }

        if (tcb[n].state == TCP_STATE_LISTEN)
        {
            if (TCP_GET_FLAGS(tcp) == TCP_FLAG_SYN)
            {
                send_reply(eth, ip, tcp, TCP_FLAG_ACK|TCP_FLAG_SYN,
                    TCP_GET_SEQNO(tcp) + 1,             // received SYN
                    tcb[n].seqno,
                    0);

                tcb[n].seqno += 1;          // sent SYN

                tcb[n].state = TCP_STATE_SYNRCVD;
            }
        }
        else
        if (tcb[n].state == TCP_STATE_SYNRCVD)
        {
            if (TCP_GET_FLAGS(tcp) == TCP_FLAG_ACK)
                tcb[n].state = TCP_STATE_ESTABLISHED;
        }
        else
        if (tcb[n].state == TCP_STATE_ESTABLISHED)
        {
            if (TCP_GET_FLAGS(tcp) & TCP_FLAG_FIN)
            {
                send_reply(eth, ip, tcp, TCP_FLAG_ACK|TCP_FLAG_FIN,
                    TCP_GET_SEQNO(tcp) + left + 1,   // received FIN + data
                    tcb[n].seqno,
                    0);

                tcb[n].seqno += 1;          // sent FIN

                tcb[n].state = TCP_STATE_LASTACK;
            }
            else
            {
                uint16_t        reply_len;

                if (TCP_GET_DST_PORT(tcp) == TCP_PROTOCOL_HTTP)
                {
                    /*
                     * Process the request, send the reply and include a FIN
                     * to end the conversation.
                     */
                    if (http_request_handler)
                        (*http_request_handler)(data, &reply_len);

                    send_reply(eth, ip, tcp, TCP_FLAG_ACK|TCP_FLAG_FIN,
                        TCP_GET_SEQNO(tcp) + left,       // received left
                        tcb[n].seqno,
                        reply_len);

                    tcb[n].seqno += reply_len + 1;  // sent reply_len + FIN

                    tcb[n].state = TCP_STATE_FINWAIT1;
                }
            }
        }
        else
        if (tcb[n].state == TCP_STATE_FINWAIT1)
        {
            if (TCP_GET_FLAGS(tcp) == (TCP_FLAG_ACK|TCP_FLAG_FIN))
            {
                send_reply(eth, ip, tcp, TCP_FLAG_ACK,
                    TCP_GET_SEQNO(tcp) + 1,         // received FIN
                    tcb[n].seqno,
                    0);

                tcb[n].state = TCP_STATE_TIMEWAIT;
            }
            else
            if (TCP_GET_FLAGS(tcp) == TCP_FLAG_ACK)
                tcb[n].state = TCP_STATE_FINWAIT2;
            else
            if (TCP_GET_FLAGS(tcp) == TCP_FLAG_FIN)
            {
                send_reply(eth, ip, tcp, TCP_FLAG_ACK,
                    TCP_GET_SEQNO(tcp) + 1,         // received FIN
                    tcb[n].seqno,
                    0);

                tcb[n].state = TCP_STATE_CLOSING;
            }
        }
        else
        if (tcb[n].state == TCP_STATE_FINWAIT2)
        {
            if (TCP_GET_FLAGS(tcp) == TCP_FLAG_FIN)
            {
                send_reply(eth, ip, tcp, TCP_FLAG_ACK,
                    TCP_GET_SEQNO(tcp) + 1,         // received FIN
                    tcb[n].seqno, 0);
                tcb[n].state = TCP_STATE_TIMEWAIT;
            }
        }
        else
        if (tcb[n].state == TCP_STATE_CLOSING)
        {
            if (TCP_GET_FLAGS(tcp) == TCP_FLAG_ACK)
                tcb[n].state = TCP_STATE_TIMEWAIT;
        }
        else
        if (tcb[n].state == TCP_STATE_TIMEWAIT)
        {
            // XXX: set timeout
            tcb[n].msl2 = clock_current_time() + 30 * 100;     // 30 seconds
        }
        else
        if (tcb[n].state == TCP_STATE_LASTACK)
        {
            if (TCP_GET_FLAGS(tcp) == TCP_FLAG_ACK)
                tcb[n].state = TCP_STATE_LISTEN;
        }

        // XXX: ignore anything else?
        return;
    }

FAIL:
    send_reply(eth, ip, tcp, TCP_FLAG_ACK|TCP_FLAG_RST, TCP_GET_SEQNO(tcp), 0, 0);

    return;
}

/****************************************************************************/
/* HTTP Layer */
/****************************************************************************/

#include "http.h"

/*
 * Define a handler to process any incoming HTTP requests
 */
void
network_set_http_request_handler(http_request_handler_t *handler)
{
    http_request_handler = handler;
}

void
network_send_http_request(uint8_t *dest_ip_address, char *message, char *useragent)
{
#if 0
    uint8_t *eth    = pkt;
    uint8_t *ip     = eth + ETH_HEADER_LEN;
    uint8_t *tcp    = ip + IP_HEADER_LEN;
    uint8_t *http   = tcp + TCP_HEADER_LEN;
    int     len;

    cli();  /* disable reply handling to prevent buffer overwrite */

    /*
     * Create the ethernet header
     */
    for (uint8_t i = 0; i < 6; i++)
    {
        eth[ETH_SRC_OFFSET+i] = mac_address[i];
        eth[ETH_DST_OFFSET+i] = 0xff;

        arp[ARP_SRC_HARDWARE_OFFSET+i] = mac_address[i];
        arp[ARP_DST_HARDWARE_OFFSET+i] = 0;
    }
    ETH_SET_TYPE(eth, ETH_PROTOCOL_ARP);

    /*
     * Create the ARP header
     */
    ARP_SET_HARDWARE(arp, ARP_HARDWARE_ETHER);
    ARP_SET_PROTOCOL(arp, ARP_PROTOCOL_IP);
    ARP_SET_HWLEN(arp, ARP_HWLEN_ETHER);
    ARP_SET_PRLEN(arp, ARP_PRLEN_IP);

    ARP_SET_OPCODE(arp, ARP_OPCODE_REQUEST);

    ARP_SET_SRC_PROTOCOL_ADDR_IP(arp, ip_address);
    ARP_SET_DST_PROTOCOL_ADDR_IP(arp, req_ip_address);

    /*
     * Send the packet!
     */
    len = ETH_HEADER_LEN + ARP_HEADER_LEN;
    enc28j60_send_packet(eth, len);

    /*
     * Add a pending cache entry
     */
    uint8_t slot    = arp_get_cache_slot();

    for (uint8_t i = 0; i < 4; i++)
        arp_cache[slot].ip_address[i] = req_ip_address[i];

    arp_cache[slot].alloc = 1;
     
    sei();
#endif
}

/****************************************************************************/
/* NTP Layer */
/****************************************************************************/

#include "ntp.h"

uint8_t ntp_request_ip[4];

static void
ntp_process_packet(uint8_t *eth, uint8_t *ip, uint8_t *udp, uint8_t *data, unsigned int left)
{
    if (left < NTP_HEADER_LEN)
        return;

    uint8_t         *ntp        = data;

    if
    (
        ntp_reply_handler != 0
        &&
        ip[IP_SRC_IP_OFFSET+0] == ntp_request_ip[0]
        &&
        ip[IP_SRC_IP_OFFSET+1] == ntp_request_ip[1]
        &&
        ip[IP_SRC_IP_OFFSET+2] == ntp_request_ip[2]
        &&
        ip[IP_SRC_IP_OFFSET+3] == ntp_request_ip[3]
        &&
        UDP_GET_SRC_PORT(udp) == UDP_PORT_NTP
        &&
        (NTP_GET_LIVNMODE(ntp) & 0x07) == 4
    )
    {
        /*
         * This is an NTP server reply from our NTP server.
         * Set it in the RTC.
         */
        (*ntp_reply_handler)(NTP_GET_TRANSMIT_SECS(ntp));

        // zero out so we don't accept any more
        ntp_request_ip[0] = 0;
        ntp_request_ip[1] = 0;
        ntp_request_ip[2] = 0;
        ntp_request_ip[3] = 0;
    }

}

static void
send_sntp_request(uint8_t *server_mac, uint8_t *server_ip)
{
    uint8_t         *eth    = pkt;
    uint8_t         *ip     = eth + ETH_HEADER_LEN;
    uint8_t         *udp    = ip + IP_HEADER_LEN;
    uint8_t         *ntp    = udp + UDP_HEADER_LEN;
    unsigned int    pktlen;

    cli();  /* disable reply handling to prevent buffer overwrite */

    pktlen = ETH_HEADER_LEN + IP_HEADER_LEN + UDP_HEADER_LEN + NTP_HEADER_LEN;
    for (int i = 0; i < pktlen; i++)
        pkt[i] = '\0';

    /*
     * Create the Ethernet header
     */
    for (uint8_t i = 0; i < 6; i++)
    {
        eth[ETH_SRC_OFFSET+i] = mac_address[i];
        eth[ETH_DST_OFFSET+i] = server_mac[i];
    }
    ETH_SET_TYPE(eth, ETH_PROTOCOL_IP);

    /*
     * Create the IP header
     */
    IP_SET_VERSION(ip, IP_VERSION_IPV4);
    IP_SET_HDR_LEN(ip, 5);
    IP_SET_TOS(ip, 0);
    IP_SET_LENGTH(ip, IP_HEADER_LEN + UDP_HEADER_LEN + NTP_HEADER_LEN);
    IP_SET_ID(ip, clock_current_time() ^ ((UDP_PORT_NTP << 8) + UDP_PORT_NTP));
    IP_SET_FRAG_FLAGS(ip, IP_FLAG_DONT_FRAGMENT);
    IP_SET_FRAG_OFFSET(ip, 0);
    IP_SET_TTL(ip, 64);
    IP_SET_PROTOCOL(ip, IP_PROTOCOL_UDP);

    ip[IP_SRC_IP_OFFSET+0] = ip_address[0];
    ip[IP_SRC_IP_OFFSET+1] = ip_address[1];
    ip[IP_SRC_IP_OFFSET+2] = ip_address[2];
    ip[IP_SRC_IP_OFFSET+3] = ip_address[3];

    ip[IP_DST_IP_OFFSET+0] = ntp_request_ip[0] = server_ip[0];
    ip[IP_DST_IP_OFFSET+1] = ntp_request_ip[1] = server_ip[1];
    ip[IP_DST_IP_OFFSET+2] = ntp_request_ip[2] = server_ip[2];
    ip[IP_DST_IP_OFFSET+3] = ntp_request_ip[3] = server_ip[3];

    /*
     * Create the UDP header
     */
    UDP_SET_SRC_PORT(udp, UDP_PORT_NTP);
    UDP_SET_DST_PORT(udp, UDP_PORT_NTP);
    UDP_SET_LENGTH(udp, UDP_HEADER_LEN + NTP_HEADER_LEN);

    /*
     * Create the NTP header
     */
    NTP_SET_LIVNMODE(ntp, ((3 << 3) + 3));

    /*
     * Build checksums
     */
    ip_make_checksum(ip);
    tcpudp_make_checksum(ip, udp, UDP_HEADER_LEN + NTP_HEADER_LEN, 0);

    /*
     * Send the packet!
     */
    enc28j60_send_packet(eth, pktlen);

    sei();
}

/*
 * Define a handler to process any incoming NTP replies
 */
void
network_set_ntp_reply_handler(ntp_reply_handler_t *handler)
{
    ntp_reply_handler = handler;
}

static uint32_t
ntp_request_retry(uint32_t now, uint32_t *data)
{
    uint8_t server_ip[4];

    server_ip[0] = (*data & 0xff000000) >> 24;
    server_ip[1] = (*data & 0x00ff0000) >> 16;
    server_ip[2] = (*data & 0x0000ff00) >> 8;
    server_ip[3] = (*data & 0x000000ff);

    arp_cache_entry_t   *c = arp_cache_lookup(server_ip);

    /*
     * Do we have the server in our ARP cache yet?  If not, then try again
     * in another 10 ticks.
     */
    if (!c || !c->valid)
    {
        return now + 10;
    }

    /*
     * We have a valid ARP entry, so send the NTP request.
     */
    send_sntp_request(c->mac_address, c->ip_address);

    return 0;
}

/*
 * Initiate an NTP request
 */
uint8_t
network_send_ntp_request(uint8_t *server_ip)
{
    arp_cache_entry_t   *c = arp_cache_lookup(server_ip);

    /*
     * Do we have the server in our ARP cache?  If not, we need to send out an
     * ARP request.
     */

    if (c && c->valid)
    {
        /*
         * We have a valid ARP cache entry, so send the NTP request
         */
        send_sntp_request(c->mac_address, c->ip_address);
    }
    else
    {
        if (!c)
        {
            /*
             * There is no pending ARP request, so send one.
             */
            send_arp_request(server_ip);
        }
        else
        {
            /*
             * There is already a pending request
             */
        }

        /*
         * Queue up a task to retry the request
         */
        uint32_t data
            = ((uint32_t)server_ip[0] << 24)
            + ((uint32_t)server_ip[1] << 16)
            + ((uint32_t)server_ip[2] <<  8)
            + ((uint32_t)server_ip[3]);

        task_submit(clock_current_time() + 10, 20, ntp_request_retry, data);
    }

    return 0;
}

#if 0
/****************************************************************************/
/* Syslog functions */
/****************************************************************************/

static void
send_syslog_message(uint8_t *server_mac, uint8_t *server_ip, unsigned char *message)
{
    uint8_t         *eth    = pkt;
    uint8_t         *ip     = eth + ETH_HEADER_LEN;
    uint8_t         *udp    = ip + IP_HEADER_LEN;
    unsigned int    pktlen;

    pkthdrlen = ETH_HEADER_LEN + IP_HEADER_LEN + UDP_HEADER_LEN;
    for (int i = 0; i < pktlen; i++)
        pkt[i] = '\0';

    /*
     * Create the Ethernet header
     */
    for (uint8_t i = 0; i < 6; i++)
    {
        eth[ETH_SRC_OFFSET+i] = mac_address[i];
        eth[ETH_DST_OFFSET+i] = server_mac[i];
    }
    ETH_SET_TYPE(eth, ETH_PROTOCOL_IP);

    /*
     * Create the IP header
     */
    ip[IP_VL_OFFSET] = (IP_VERSION_IPV4 << 4) + 5;
    IP_TOS(ip) = 0;

    uint16_t len = IP_HEADER_LEN + UDP_HEADER_LEN + NTP_HEADER_LEN;
    IP_LENGTH_H(ip) = (len & 0xff00) >> 8;
    IP_LENGTH_L(ip) = (len & 0x00ff);

    IP_SET_ID(clock_current_time() ^ ((UDP_PORT_NTP << 8) + UDP_PORT_NTP));

    uint16_t frag;
    frag = (IP_FLAG_DONT_FRAGMENT << 13) + 0x0000;
    IP_FRAGMENT_H(ip) = (frag & 0xff00) >> 8;
    IP_FRAGMENT_L(ip) = (frag & 0x00ff);

    IP_TTL(ip) = 64;

    IP_PROTOCOL(ip) = IP_PROTOCOL_UDP;

    ip[IP_SRC_IP_OFFSET+0] = ip_address[0];
    ip[IP_SRC_IP_OFFSET+1] = ip_address[1];
    ip[IP_SRC_IP_OFFSET+2] = ip_address[2];
    ip[IP_SRC_IP_OFFSET+3] = ip_address[3];

    ip[IP_DST_IP_OFFSET+0] = ntp_request_ip[0] = server_ip[0];
    ip[IP_DST_IP_OFFSET+1] = ntp_request_ip[1] = server_ip[1];
    ip[IP_DST_IP_OFFSET+2] = ntp_request_ip[2] = server_ip[2];
    ip[IP_DST_IP_OFFSET+3] = ntp_request_ip[3] = server_ip[3];

    /*
     * Create the UDP header
     */
    UDP_SET_SRC_PORT(udp, UDP_PORT_NTP);

    UDP_SET_DST_PORT(udp, UDP_PORT_NTP);

    UDP_SET_LENGTH(udp, UDP_HEADER_LEN + NTP_HEADER_LEN);

    /*
     * Create the NTP header
     */
    NTP_SET_LIVNMODE(ntp, ((3 << 3) + 3));

    /*
     * Build checksums
     */
    ip_make_checksum(ip);
    tcpudp_make_checksum(ip, udp, UDP_HEADER_LEN + NTP_HEADER_LEN, 0);

    /*
     * Send the packet!
     */
    enc28j60_send_packet(eth, pktlen);

    sei();
}

static uint32_t
syslog_message_retry(uint32_t now, uint32_t *data)
{
    uint8_t server_ip[4];

    server_ip[0] = (*data & 0xff000000) >> 24;
    server_ip[1] = (*data & 0x00ff0000) >> 16;
    server_ip[2] = (*data & 0x0000ff00) >> 8;
    server_ip[3] = (*data & 0x000000ff);

    arp_cache_entry_t   *c = arp_cache_lookup(server_ip);

    /*
     * Do we have the server in our ARP cache yet?  If not, then try again
     * in another 10 ticks.
     */
    if (!c || !c->valid)
    {
        return now + 10;
    }

    /*
     * We have a valid ARP entry, so send the NTP request.
     */
    send_syslog_message(c->mac_address, c->ip_address);

    return 0;
}

/*
 * Send a SYSLOG packet
 */
uint8_t
network_send_syslog(uint8_t *server_ip, uint8_t facility, uint8_t severity,
    unsigned char *message)
{
    arp_cache_entry_t   *c = arp_cache_lookup(server_ip);

    /*
     * Do we have the server in our ARP cache?  If not, we need to send out an
     * ARP request.
     */

    if (c && c->valid)
    {
        /*
         * We have a valid ARP cache entry, so send the NTP request
         */
        send_syslog_message(c->mac_address, c->ip_address);
    }
    else
    {
        if (!c)
        {
            /*
             * There is no pending ARP request, so send one.
             */
            send_syslog_message(server_ip);
        }
        else
        {
            /*
             * There is already a pending request
             */
        }

        /*
         * Queue up a task to retry the request
         */
        uint32_t data
            = ((uint32_t)server_ip[0] << 24)
            + ((uint32_t)server_ip[1] << 16)
            + ((uint32_t)server_ip[2] <<  8)
            + ((uint32_t)server_ip[3]);

        task_submit(clock_current_time() + 10, 20, syslog_message_retry, data);
    }

    return 0;
}
#endif /* 0 */


/****************************************************************************/
/* Miscellaneous functions */
/****************************************************************************/

void
network_set_mac_address(uint8_t *mac)
{
    /*
     * Set the address in the Ethernet layer
     */
    eth_set_address(mac);

    /* 
     * Define the address in the ENC28J60 hardware
     */
    enc28j60_set_mac_address(mac);
}

void
network_set_ip_address(uint8_t *ip)
{
    ip_set_address(ip);
}

void
network_read_packet(void)
{
    enc28j60_read_packet(pkt, AVR_FEATURE_NWSTACK_PKT_BUFFER_SIZE);
}

void
network_init(gpio_line_t *slave_select)
{
    /*
     * Set up the ENC28J60 hardware
     */
    enc28j60_init(slave_select, &eth_process_packet);

    /*
     * Initialise various protocol layers
     */
    arp_cache_init();
}
