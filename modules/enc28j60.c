#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "avr-common.h"
#include MCU_H
#include "spi.h"
#include "enc28j60.h"

#define FULL_DUPLEX

static raw_handler_t    *incoming_pkt_handler;
static gpio_line_t      ss_port;

/*
 * The current register bank
 */
static uint8_t  current_bank   = 0;

static void
_enc28j60_set_bank(regcode_t regcode)
{
    uint8_t reg     = REGCODE_REGISTER(regcode);
    uint8_t bank    = REGCODE_BANK(regcode);

    /*
     * Switch banks if this register is in a different bank, and it is not
     * a register common to all banks.
     */
    if (current_bank != bank && reg < FIRST_COMMON_REGISTER)
    {
        uint8_t econ1;

        /*
         * Need to switch banks by setting bits 1..0 of ECON1
         */
        spi_start_tx(&ss_port);
        spi_send_byte( INSTR_RCR(ECON1) );
        econ1 = spi_receive_byte();
        spi_end_tx(&ss_port);

        econ1 = (econ1 & 0xfc) | (bank & 0x03);

        spi_start_tx(&ss_port);
        spi_send_byte( INSTR_WCR(ECON1) );
        spi_send_byte( econ1 );
        spi_end_tx(&ss_port);

        current_bank = bank;
    }
}

/*
 * Perform a SRC (System Reset Command)
 */
void
enc28j60_src(void)
{
    spi_start_tx(&ss_port);
    spi_send_byte( INSTR_SRC );
    spi_end_tx(&ss_port);
}

/*
 * Perform a RCR request, returning the value from the given register
 */
uint8_t
enc28j60_rcr(regcode_t regcode)
{
    uint8_t reg     = REGCODE_REGISTER(regcode);
    uint8_t v;

    _enc28j60_set_bank(regcode);

    /*
     * Issue the RCR instruction and read the reply. "Extended" registers
     * send a dummy byte before the real value.
     */
    spi_start_tx(&ss_port);
    spi_send_byte( INSTR_RCR(reg) );
    v = spi_receive_byte();
    if (REGCODE_EXTENDED(regcode))
    {
        // previous was a dummy byte
        v = spi_receive_byte();
    }
    spi_end_tx(&ss_port);

    return v;
}

static void
enc28j60_void_op(regcode_t regcode, uint8_t op, uint8_t value)
{
    _enc28j60_set_bank(regcode);

    /*
     * Issue the instruction.
     */
    spi_start_tx(&ss_port);
    spi_send_byte( op );
    spi_send_byte( value );
    spi_end_tx(&ss_port);
}

/*
 * Perform a WCR request, updating given register with the new value
 */
void
enc28j60_wcr(regcode_t regcode, uint8_t value)
{
    uint8_t reg     = REGCODE_REGISTER(regcode);

    enc28j60_void_op(regcode, INSTR_WCR(reg), value);
}

/*
 * Perform a BFS request, setting bits in the given ETH register
 */
void
enc28j60_bfs(regcode_t regcode, uint8_t bits)
{
    uint8_t reg     = REGCODE_REGISTER(regcode);

    enc28j60_void_op(regcode, INSTR_BFS(reg), bits);
}

/*
 * Perform a BFC request, clearing bits in the given ETH register
 */
void
enc28j60_bfc(regcode_t regcode, uint8_t bits)
{
    uint8_t reg     = REGCODE_REGISTER(regcode);

    enc28j60_void_op(regcode, INSTR_BFC(reg), bits);
}

/*
 * Read a PHY register
 */
uint16_t
enc28j60_rpr(regcode_t regcode)
{
    uint8_t reg     = REGCODE_REGISTER(regcode);

    // select the register to read
    enc28j60_wcr(MIREGADR, reg);

    // send the request to the PHY
    uint8_t micmd = enc28j60_rcr(MICMD);
    enc28j60_wcr(MICMD, micmd | MIIRD);

    // wait until request is complete
    while (enc28j60_rcr(MISTAT) & BUSY)
        continue;

    // reset request bit
    enc28j60_wcr(MICMD, micmd);

    // read 16-bit result
    uint8_t mirdl = enc28j60_rcr(MIRDL);
    uint8_t mirdh = enc28j60_rcr(MIRDH);

    return (mirdh << 8) + mirdl;
}

/*
 * Write a PHY register
 */
void
enc28j60_wpr(regcode_t regcode, uint16_t value)
{
    uint8_t reg     = REGCODE_REGISTER(regcode);

    // select the register to read
    enc28j60_wcr(MIREGADR, reg);

    enc28j60_wcr(MIWRL, value & 0x00ff);
    enc28j60_wcr(MIWRH, (value & 0xff00) >> 8);

    // wait until request is complete
    while (enc28j60_rcr(MISTAT) & BUSY)
        continue;
}


#ifdef UNUSED_CODE
/*
 * Perform a RBM request, returning the next byte read from the buffer
 *
 * XXX: This is not the most efficent way read a whole packet, as we can
 * keep clocking out packets.
 */
uint8_t
enc28j60_rbm(void)
{
    uint8_t v;

    /*
     * Issue the RBM instruction and read the reply.
     */
    spi_start_tx(&ss_port);
    spi_send_byte( INSTR_RBM );
    v = spi_receive_byte();
    spi_end_tx(&ss_port);

    return v;
}
#endif /* UNUSED_CODE */

void
enc28j60_send_packet2(uint8_t *pkt1, unsigned int len1, uint8_t *pkt2, unsigned int len2)
{
    enc28j60_wcr(ETXSTL, TX_BUF_START & 0x00ff);
    enc28j60_wcr(ETXSTH, (TX_BUF_START & 0xff00) >> 8);

    enc28j60_wcr(EWRPTL, TX_BUF_START & 0x00ff);
    enc28j60_wcr(EWRPTH, (TX_BUF_START & 0xff00) >> 8);

#if 0
    uint16_t txptr = TX_BUF_START + 14 + len - 1;   // last byte in payload
#endif
    uint16_t txptr = TX_BUF_START + len1 + len2;

    enc28j60_wcr(ETXNDL, txptr & 0x00ff);
    enc28j60_wcr(ETXNDH, (txptr & 0xff00) >> 8);

    spi_start_tx(&ss_port);
    spi_send_byte( INSTR_WBM );

    // control byte
    spi_send_byte(0x00);

    // packet data
    for (int i = 0; i < len1; i++)
        spi_send_byte(pkt1[i]);

    if (pkt2)
    {
        for (int i = 0; i < len2; i++)
            spi_send_byte(pkt2[i]);
    }

    spi_end_tx(&ss_port);

    // set ECON1.TXRTS to start transmission
    enc28j60_bfs(ECON1, TXRTS);

    // wait for transmission to complete
    while (enc28j60_rcr(ECON1) & TXRTS)
        continue;

#if 0
    // check TSV
    uint8_t tsv[7];

    enc28j60_wcr(ERDPTL, (txptr+1) & 0x00ff);
    enc28j60_wcr(ERDPTH, ((txptr+1) & 0xff00) >> 8);

    spi_start_tx(&ss_port);
    spi_send_byte( INSTR_RBM );

    for (int i = 0; i < 7; i++)
        tsv[i] = spi_receive_byte();

    spi_end_tx(&ss_port);

extern char *print_dec(uint32_t v);
    if (tsv[2] & 0x80) puts("done");
    if (tsv[2] & 0x40) puts("loor");
    puts(print_dec(len));

/*
extern char *print_dec(uint32_t v);

    puts("tsv: ");
    for (int i = 0; i < 7; i++)
        puts(print_dec(tsv[i]));
    puts("\n");
*/

#endif
}

void
enc28j60_send_packet(uint8_t *pkt, unsigned int len)
{
    enc28j60_send_packet2(pkt, len, 0, 0);
}


#if DUMP
static void
enc28j60_dump_mac(void)
{
    uint16_t  v;

    printf("DUMP MAC REGISTERS\n");

    v = enc28j60_rcr(MACON1);
    printf("  MACON1: ");
    if (v & TXPAUS)     printf("TXPAUS ");
    if (v & RXPAUS)     printf("RXPAUS ");
    if (v & PASSALL)    printf("PASSALL ");
    if (v & MARXEN)     printf("MARXEN ");
    printf("\n");

    v = enc28j60_rcr(MACON3);
    printf("  MACON3: ");
    if (v & PADCFG2)    printf("PADCFG2 ");
    if (v & PADCFG1)    printf("PADCFG1 ");
    if (v & PADCFG0)    printf("PADCFG0 ");
    if (v & TXCRCEN)    printf("TXCRCEN ");
    if (v & PHDREN)     printf("PHDREN ");
    if (v & HFRMEN)     printf("HFRMEN ");
    if (v & FRMLNEN)    printf("FRMLNEN ");
    if (v & FULDPX)     printf("FULDPX ");
    printf("\n");

    v = enc28j60_rcr(MACON4);
    printf("  MACON4: ");
    if (v & DEFER)      printf("DEFER ");
    if (v & BPEN)       printf("BPEN ");
    if (v & NOBKOFF)    printf("NOBKOFF ");
    printf("\n");

    printf("  MABBIPG.BBIPG = %02x\n",
        enc28j60_rcr(MABBIPG) & 0x7f);

    printf("  MAIPG = %02x%02x\n",
        enc28j60_rcr(MAIPGH) & 0x7f,
        enc28j60_rcr(MAIPGL) & 0x7f);

    printf("  RETMAX = %02x\n",
        enc28j60_rcr(MACLCON1) & 0x0f);

    printf("  COLWIN = %02x\n",
        enc28j60_rcr(MACLCON2) & 0x3f);

    printf("  MAMXFL = %02x%02x\n",
        enc28j60_rcr(MAMXFLH),
        enc28j60_rcr(MAMXFLL));

    printf("  MAADR = %02x.%02x.%02x.%02x.%02x.%02x\n",
        enc28j60_rcr(MADR1),
        enc28j60_rcr(MADR2),
        enc28j60_rcr(MADR3),
        enc28j60_rcr(MADR4),
        enc28j60_rcr(MADR5),
        enc28j60_rcr(MADR6));
}

static void
enc28j60_dump_phy(void)
{
    uint16_t  v;

    printf("DUMP PHY REGISTERS\n");

    v = enc28j60_rpr(PHCON1);
    printf("  PHCON1: ");
    if (v & PRST)       printf("PRST ");
    if (v & PLOOPBK)    printf("PLOOPBK ");
    if (v & PPWRSV)     printf("PPWRSV ");
    if (v & PDPXMD)     printf("PDPXMD ");
    printf("\n");

    v = enc28j60_rpr(PHSTAT1);
    printf("  PHSTAT1: ");
    if (v & PFDPX)      printf("PFDPX ");
    if (v & PHDPX)      printf("PHDPX ");
    if (v & LLSTAT)     printf("LLSTAT ");
    if (v & JBSTAT)     printf("JBSTAT ");
    printf("\n");

    printf("  PHID1 = %04x\n",
        enc28j60_rpr(PHID1));

    v = enc28j60_rpr(PHID2);
    printf("  PHID2 = %02x %02x %02x\n",
        (v & 0xfc00) >> 10,
        (v & 0x03f0) >> 4,
        (v & 0x000f));

    v = enc28j60_rpr(PHCON2);
    printf("  PHCON2: ");
    if (v & FRCLNK)     printf("FRCLNK ");
    if (v & TXDIS)      printf("TXDIS ");
    if (v & JABBER)     printf("JABBER ");
    if (v & HDLDIS)     printf("HDLDIS ");
    printf("\n");

    v = enc28j60_rpr(PHSTAT2);
    printf("  PHSTAT2: ");
    if (v & TXSTAT)     printf("TXSTAT ");
    if (v & RXSTAT)     printf("RXSTAT ");
    if (v & COLSTAT)    printf("COLSTAT ");
    if (v & LSTAT)      printf("LSTAT ");
    if (v & DPXSTAT)    printf("DPXSTAT ");
    if (v & PLRITY)     printf("PLRITY ");
    printf("\n");

    v = enc28j60_rpr(PHIE);
    printf("  PHIE: ");
    if (v & PLNKIE)     printf("PLNKIE ");
    if (v & PGEIE)      printf("PGEIE ");
    printf("\n");

    v = enc28j60_rpr(PHHIR);
    printf("  PHHIR: ");
    if (v & PLNKIF)     printf("PLNKIF ");
    if (v & PGIF)       printf("PGIF ");
    printf("\n");

    v = enc28j60_rpr(PHLCON);
    printf("  PHLCON: ");
    if (v & LACFG3)     printf("LACFG3 ");
    if (v & LACFG2)     printf("LACFG2 ");
    if (v & LACFG1)     printf("LACFG1 ");
    if (v & LACFG0)     printf("LACFG0 ");
    if (v & LBCFG3)     printf("LBCFG3 ");
    if (v & LBCFG2)     printf("LBCFG2 ");
    if (v & LBCFG1)     printf("LBCFG1 ");
    if (v & LBCFG0)     printf("LBCFG0 ");
    if (v & LFRQ1)      printf("LFRQ1 ");
    if (v & LFRQ0)      printf("LFRQ0 ");
    if (v & STRCH)      printf("STRCH ");
    printf("\n");
}
#endif /* DUMP */

void
enc28j60_set_mac_address(uint8_t *mac)
{
    // define MAC address
    enc28j60_wcr(MADR1, mac[0]);
    enc28j60_wcr(MADR2, mac[1]);
    enc28j60_wcr(MADR3, mac[2]);
    enc28j60_wcr(MADR4, mac[3]);
    enc28j60_wcr(MADR5, mac[4]);
    enc28j60_wcr(MADR6, mac[5]);

    // XXX: set filters
}

uint16_t
enc28j60_read_packet(uint8_t *pkt, uint16_t maxlen)
{
    if (enc28j60_rcr(EPKTCNT) == 0)
        return 0;

    enc28j60_bfc(EIE, INTIE);

    spi_start_tx(&ss_port);
    spi_send_byte( INSTR_RBM );

    /*
     * Read in the received packet metadata
     */
    uint8_t nxtptr0 = spi_receive_byte();
    uint8_t nxtptr1 = spi_receive_byte();

    uint8_t rsv0 = spi_receive_byte();
    uint8_t rsv1 = spi_receive_byte();
#if 0
    uint8_t rsv2 = spi_receive_byte();
    uint8_t rsv3 = spi_receive_byte();
#else
    spi_receive_byte();
    spi_receive_byte();
#endif

    /*
     * Read in up to 256 bytes of packet data
     */
    unsigned int    bytes_left      = (rsv1 << 8) + rsv0;
    unsigned int    bytes_to_read   = bytes_left < maxlen ? bytes_left : maxlen;

    for (int i = 0; i < bytes_to_read; i++)
        pkt[i] = spi_receive_byte();

    spi_end_tx(&ss_port);

    /*
     * Handle raw packet encapsulation
     */
    if (incoming_pkt_handler)
        (*incoming_pkt_handler)(pkt, bytes_to_read);

    /*
     * Update the RX buffer pointers
     */
    enc28j60_wcr(ERDPTL, nxtptr0);
    enc28j60_wcr(ERDPTH, nxtptr1);

    enc28j60_wcr(ERXRDPTL, nxtptr0);
    enc28j60_wcr(ERXRDPTH, nxtptr1);

    enc28j60_bfs(ECON2, PKTDEC);

    enc28j60_bfs(EIE, INTIE);

    return bytes_to_read;
}

void
enc28j60_init(gpio_line_t *slave_select, raw_handler_t *pkt_handler)
{
    ss_port = *slave_select;

    spi_init();
    spi_init_slave(&ss_port);

    /*
     * Reset the ethernet hardware
     */
    enc28j60_src();


    /*
     * Define the receive/transmit buffers
     */
    enc28j60_wcr(ERXSTL, RX_BUF_START & 0x00ff);
    enc28j60_wcr(ERXSTH, (RX_BUF_START & 0xff00) >> 8);

    enc28j60_wcr(ERXNDL, RX_BUF_END & 0x00ff);
    enc28j60_wcr(ERXNDH, (RX_BUF_END & 0xff00) >> 8);

    enc28j60_wcr(ERDPTL, RX_BUF_START & 0x00ff);
    enc28j60_wcr(ERDPTH, (RX_BUF_START & 0xff00) >> 8);

    enc28j60_wcr(ERXRDPTL, RX_BUF_START & 0x00ff);
    enc28j60_wcr(ERXRDPTH, (RX_BUF_START & 0xff00) >> 8);

    enc28j60_wcr(ERXWRPTL, RX_BUF_START & 0x00ff);
    enc28j60_wcr(ERXWRPTH, (RX_BUF_START & 0xff00) >> 8);

    enc28j60_bfs(ECON2, AUTOINC);

    /*
     * Set up the receive filters
     *
     * - unicast or broadcast with valid CRC
     */
    enc28j60_wcr(ERXFCON, UCEN|CRCEN|BCEN);

    /*
     * Wait for OST
     */
    _delay_ms(1);
    while (!(enc28j60_rcr(ESTAT) & CLKRDY))
        continue;

    /*
     * MAC Initialisation
     */
#ifdef FULL_DUPLEX
    enc28j60_wcr(MACON1, MARXEN|TXPAUS|RXPAUS);   // FDUPLEX
    enc28j60_wcr(MACON3, PADCFG0|TXCRCEN|FRMLNEN|FULDPX);  // FDUPLEX
    enc28j60_wcr(MACON4, 0);
#else
    enc28j60_wcr(MACON1, MARXEN);   // HDUPLEX
    enc28j60_wcr(MACON3, PADCFG2|PADCFG1|PADCFG0|TXCRCEN|FRMLNEN);   // HDUPLEX
    enc28j60_wcr(MACON4, DEFER);
#endif

#define MAX_FRAME_LEN   1500

    enc28j60_wcr(MAMXFLL, (MAX_FRAME_LEN & 0x00ff));
    enc28j60_wcr(MAMXFLH, (MAX_FRAME_LEN & 0xff00) >> 8);

#ifdef FULL_DUPLEX
    enc28j60_wcr(MABBIPG, 0x15);    // FDUPLEX
    enc28j60_wcr(MAIPGL, 0x12);
#else
    enc28j60_wcr(MABBIPG, 0x12);    // HDUPLEX
    enc28j60_wcr(MAIPGL, 0x12);
    enc28j60_wcr(MAIPGH, 0x0c);  // HDUPLEX
#endif

    /*
    enc28j60_set_mac_address(mac);
    */

    // enc28j60_dump_mac();

#ifdef FULL_DUPLEX
    enc28j60_wpr(PHCON1, enc28j60_rpr(PHCON1) | PDPXMD);
#else
    enc28j60_wpr(PHCON1, enc28j60_rpr(PHCON1) & ~PDPXMD);
#endif

    // enc28j60_dump_phy();

    // LED2 shows duplex status
    // enc28j60_wpr(PHLCON, (enc28j60_rpr(PHLCON) & ~(LBCFG3|LBCFG1)) | LBCFG2|LBCFG0);

    incoming_pkt_handler = pkt_handler;

    /*
     * Set up interrupts. On packet receipt, clear the INT pin.
     */
    enc28j60_bfs(EIE, INTIE|PKTIE);

    // enable the receiver
    enc28j60_bfs(ECON1, RXEN);
}
