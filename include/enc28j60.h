#ifndef __INCLUDE_ENC28J60_H
#define __INCLUDE_ENC28J60_H

#include "avr-common.h"

/*
 * Receive/transmit buffers
 */
#define RX_BUF_START        0x0000
#define RX_BUF_END          0x17ff

#define TX_BUF_START        0x1800
#define TX_BUF_END          0x1fff

/*
 * Instruction macros
 */
#define INSTR_RCR(reg)      (0x00 + ((reg) & 0x1f))
#define INSTR_RBM           (0x3a)
#define INSTR_WCR(reg)      (0x40 + ((reg) & 0x1f))
#define INSTR_WBM           (0x7a)
#define INSTR_BFS(reg)      (0x80 + ((reg) & 0x1f))
#define INSTR_BFC(reg)      (0xa0 + ((reg) & 0x1f))
#define INSTR_SRC           (0xff)

/*
 * Each register #define also encodes 2 bits for the bank and 1 bit if it
 * is a MAC or MII register (different read semantics).
 */
#define REG_MASK_REGISTER   0x1f
#define REG_MASK_BANK       0x60
#define REG_MASK_EXTEND     0x80

#define REG_SHIFT_BANK      5
#define REG_SHIFT_EXTEND    7

#define REGCODE_REGISTER(reg)   ((reg) & REG_MASK_REGISTER)
#define REGCODE_BANK(reg)       (((reg) & REG_MASK_BANK) >> REG_SHIFT_BANK)
#define REGCODE_EXTENDED(reg)   (((reg) & REG_MASK_EXTEND) >> REG_SHIFT_EXTEND)

// bit values for each bank
#define F_BANK0     (0<<REG_SHIFT_BANK)
#define F_BANK1     (1<<REG_SHIFT_BANK)
#define F_BANK2     (2<<REG_SHIFT_BANK)
#define F_BANK3     (3<<REG_SHIFT_BANK)

// bit value for extended register
#define F_EXTEND    (1<<REG_SHIFT_EXTEND)

/*
 * Define the various (encoded) register values
 */

// Bank 0
#define ERDPTL      (F_BANK0|0x00)
#define ERDPTH      (F_BANK0|0x01)
#define EWRPTL      (F_BANK0|0x02)
#define EWRPTH      (F_BANK0|0x03)
#define ETXSTL      (F_BANK0|0x04)
#define ETXSTH      (F_BANK0|0x05)
#define ETXNDL      (F_BANK0|0x06)
#define ETXNDH      (F_BANK0|0x07)
#define ERXSTL      (F_BANK0|0x08)
#define ERXSTH      (F_BANK0|0x09)
#define ERXNDL      (F_BANK0|0x0a)
#define ERXNDH      (F_BANK0|0x0b)
#define ERXRDPTL    (F_BANK0|0x0c)
#define ERXRDPTH    (F_BANK0|0x0d)
#define ERXWRPTL    (F_BANK0|0x0e)
#define ERXWRPTH    (F_BANK0|0x0f)
#define EDMASTL     (F_BANK0|0x10)
#define EDMASTH     (F_BANK0|0x11)
#define EDMANDL     (F_BANK0|0x12)
#define EDMANDH     (F_BANK0|0x13)
#define EDMADSTL    (F_BANK0|0x14)
#define EDMADSTH    (F_BANK0|0x15)
#define EDMACTL     (F_BANK0|0x16)
#define EDMACTH     (F_BANK0|0x17)

// Common
#define EIE         (F_BANK0|0x1b)
#define     INTIE       (1<<7)
#define     PKTIE       (1<<6)
#define     DMAIE       (1<<5)
#define     LINKIE      (1<<4)
#define     TXIE        (1<<3)
#define     TXERIE      (1<<1)
#define     RXERIE      (1<<0)
#define EIR         (F_BANK0|0x1c)
#define     PKTIF       (1<<6)
#define     DMAIF       (1<<5)
#define     LINKIF      (1<<4)
#define     TXIF        (1<<3)
#define     TXERIF      (1<<1)
#define     RXERIF      (1<<0)
#define ESTAT       (F_BANK0|0x1d)
#define     INT         (1<<7)
#define     BUFER       (1<<6)
#define     LATECOL     (1<<4)
#define     RXBUSY      (1<<2)
#define     TXABRT      (1<<1)
#define     CLKRDY      (1<<0)
#define ECON2       (F_BANK0|0x1e)
#define     AUTOINC     (1<<7)
#define     PKTDEC      (1<<6)
#define     PWRSV       (1<<5)
#define     VRPS        (1<<3)
#define ECON1       (F_BANK0|0x1f)
#define     TXRST       (1<<7)
#define     RXRST       (1<<6)
#define     DMAST       (1<<5)
#define     CSUMEN      (1<<4)
#define     TXRTS       (1<<3)
#define     RXEN        (1<<2)
#define     BSEL1       (1<<1)
#define     BSEL0       (1<<0)

// Bank 1
#define ERXFCON     (F_BANK1|0x18)
#define     UCEN        (1<<7)
#define     ANDOR       (1<<6)
#define     CRCEN       (1<<5)
#define     PMEN        (1<<4)
#define     MPEN        (1<<3)
#define     HTEN        (1<<2)
#define     MCEN        (1<<1)
#define     BCEN        (1<<0)

#define EPKTCNT     (F_BANK1|0x19)

// Bank 2
#define MACON1      (F_BANK2|0x00|F_EXTEND)
#define     TXPAUS      (1<<3)
#define     RXPAUS      (1<<2)
#define     PASSALL     (1<<1)
#define     MARXEN      (1<<0)
#define MACON3      (F_BANK2|0x02|F_EXTEND)
#define     PADCFG2     (1<<7)
#define     PADCFG1     (1<<6)
#define     PADCFG0     (1<<5)
#define     TXCRCEN     (1<<4)
#define     PHDREN      (1<<3)
#define     HFRMEN      (1<<2)
#define     FRMLNEN     (1<<1)
#define     FULDPX      (1<<0)
#define MACON4      (F_BANK2|0x03|F_EXTEND)
#define     DEFER       (1<<6)
#define     BPEN        (1<<5)
#define     NOBKOFF     (1<<4)
#define MABBIPG     (F_BANK2|0x04|F_EXTEND)
#define MAIPGL      (F_BANK2|0x06|F_EXTEND)
#define MAIPGH      (F_BANK2|0x07|F_EXTEND)
#define MACLCON1    (F_BANK2|0x08|F_EXTEND)
#define MACLCON2    (F_BANK2|0x09|F_EXTEND)
#define MAMXFLL     (F_BANK2|0x0a|F_EXTEND)
#define MAMXFLH     (F_BANK2|0x0b|F_EXTEND)
#define MICMD       (F_BANK2|0x12|F_EXTEND)
#define     MIISCAN     (1<<1)
#define     MIIRD       (1<<0)
#define MIREGADR    (F_BANK2|0x14|F_EXTEND)
#define MIWRL       (F_BANK2|0x16|F_EXTEND)
#define MIWRH       (F_BANK2|0x17|F_EXTEND)
#define MIRDL       (F_BANK2|0x18|F_EXTEND)
#define MIRDH       (F_BANK2|0x19|F_EXTEND)

// Bank 3
#define MADR5       (F_BANK3|0x00|F_EXTEND)
#define MADR6       (F_BANK3|0x01|F_EXTEND)
#define MADR3       (F_BANK3|0x02|F_EXTEND)
#define MADR4       (F_BANK3|0x03|F_EXTEND)
#define MADR1       (F_BANK3|0x04|F_EXTEND)
#define MADR2       (F_BANK3|0x05|F_EXTEND)
#define EBSTSD      (F_BANK3|0x06)
#define EBSTCON     (F_BANK3|0x07)
#define EBSTCSL     (F_BANK3|0x08)
#define EBSTCSH     (F_BANK3|0x09)
#define MISTAT      (F_BANK3|0x0a|F_EXTEND)
#define     NVALID      (1<<2)
#define     SCAN        (1<<1)
#define     BUSY        (1<<0)
#define EREVID      (F_BANK3|0x12)
#define ECOCON      (F_BANK3|0x15)
#define EFLOCON     (F_BANK3|0x17)
#define EPAUSL      (F_BANK3|0x18)
#define EPAUSH      (F_BANK3|0x19)

/*
 * Registers >= 0x1b are common to all banks
 */
#define FIRST_COMMON_REGISTER   0x1b    // EIE

/*
 * PHY registers
 */
#define PHCON1      0x00
#define     PRST        (1<<15)
#define     PLOOPBK     (1<<14)
#define     PPWRSV      (1<<11)
#define     PDPXMD      (1<<8)
#define PHSTAT1     0x01
#define     PFDPX       (1<<12)
#define     PHDPX       (1<<11)
#define     LLSTAT      (1<<2)
#define     JBSTAT      (1<<1)
#define PHID1       0x02
#define PHID2       0x03
#define PHCON2      0x10
#define     FRCLNK      (1<<14)
#define     TXDIS       (1<<13)
#define     JABBER      (1<<10)
#define     HDLDIS      (1<<8)
#define PHSTAT2     0x11
#define     TXSTAT      (1<<13)
#define     RXSTAT      (1<<12)
#define     COLSTAT     (1<<11)
#define     LSTAT       (1<<10)
#define     DPXSTAT     (1<<9)
#define     PLRITY      (1<<5)
#define PHIE        0x12
#define     PLNKIE      (1<<4)
#define     PGEIE       (1<<1)
#define PHHIR       0x13
#define     PLNKIF      (1<<4)
#define     PGIF        (1<<2)
#define PHLCON      0x14
#define     LACFG3      (1<<11)
#define     LACFG2      (1<<10)
#define     LACFG1      (1<<9)
#define     LACFG0      (1<<8)
#define     LBCFG3      (1<<7)
#define     LBCFG2      (1<<6)
#define     LBCFG1      (1<<5)
#define     LBCFG0      (1<<4)
#define     LFRQ1       (1<<3)
#define     LFRQ0       (1<<2)
#define     STRCH       (1<<2)

typedef unsigned char   regcode_t;

typedef void (raw_handler_t)(uint8_t *pkt, uint16_t bytes);

extern void
enc28j60_src(void);

extern uint8_t
enc28j60_rcr(regcode_t regcode);

extern uint16_t
enc28j60_rpr(regcode_t regcode);

extern void
enc28j60_wpr(regcode_t regcode, uint16_t value);

extern uint8_t
enc28j60_rbm(void);

extern void
enc28j60_dump_mac(void);

extern void
enc28j60_dump_phy(void);

extern void
enc28j60_send_packet2(uint8_t *pkt1, unsigned int len1, uint8_t *pkt2, unsigned int len2);

extern void
enc28j60_send_packet(uint8_t *pkt, unsigned int len);

extern void
enc28j60_bfs(regcode_t regcode, uint8_t bits);

extern void
enc28j60_bfc(regcode_t regcode, uint8_t bits);

extern void
enc28j60_wcr(regcode_t regcode, uint8_t bits);

extern void
enc28j60_set_mac_address(uint8_t *mac);

extern void
enc28j60_init(gpio_line_t *slave_select, raw_handler_t *pkt_handler);

extern uint16_t
enc28j60_read_packet(uint8_t *pkt, uint16_t maxlen);

extern void
enc28j60_register_packet_handler(raw_handler_t *pkt_fn);

#endif /* __INCLUDE_ENC28J60_H */
