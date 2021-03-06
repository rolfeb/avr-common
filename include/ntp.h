#ifndef __INCLUDE_NTP_H
#define __INCLUDE_NTP_H

#include <stdint.h>

/*
 * NTP header definitions
 */
#define NTP_HEADER_LEN              48

#define NTP_GET_LIVNMODE(NTP)       READ_BYTE(NTP, 0)
#define NTP_SET_LIVNMODE(NTP, V)    WRITE_BYTE(NTP, 0, V)

#define NTP_GET_STRATUM(NTP)        READ_BYTE(NTP, 1)
#define NTP_SET_STRATUM(NTP, V)     WRITE_BYTE(NTP, 1, V)

#define NTP_GET_POLL(NTP)           READ_BYTE(NTP, 2)
#define NTP_SET_POLL(NTP, V)        WRITE_BYTE(NTP, 2, V)

#define NTP_GET_PRECISION(NTP)      READ_BYTE(NTP, 3)
#define NTP_SET_PRECISION(NTP, V)   WRITE_BYTE(NTP, 3, V)

#define NTP_GET_ROOTDELAY(NTP)      READ_4BYTE(NTP, 4)
#define NTP_SET_ROOTDELAY(NTP, V)   WRITE_4BYTE(NTP, 4, V)

#define NTP_GET_ROOTDISP(NTP)       READ_4BYTE(NTP, 8)
#define NTP_SET_ROOTDISP(NTP, V)    WRITE_4BYTE(NTP, 8, V)

#define NTP_GET_REFID(NTP)          READ_4BYTE(NTP, 12)
#define NTP_SET_REFID(NTP, V)       WRITE_4BYTE(NTP, 12, V)

#define NTP_GET_REFERENCE_SECS(NTP)     READ_4BYTE(NTP, 16)
#define NTP_SET_REFERENCE_SECS(NTP, V)  WRITE_4BYTE(NTP, 16, V)

#define NTP_GET_REFERENCE_FRACT(NTP)    READ_4BYTE(NTP, 20)
#define NTP_SET_REFERENCE_FRACT(NTP, V) WRITE_4BYTE(NTP, 20, V)

#define NTP_GET_ORIGINATE_SECS(NTP)     READ_4BYTE(NTP, 24)
#define NTP_SET_ORIGINATE_SECS(NTP, V)  WRITE_4BYTE(NTP, 24, V)

#define NTP_GET_ORIGINATE_FRACT(NTP)    READ_4BYTE(NTP, 28)
#define NTP_SET_ORIGINATE_FRACT(NTP, V) WRITE_4BYTE(NTP, 28, V)

#define NTP_GET_RECEIVE_SECS(NTP)       READ_4BYTE(NTP, 32)
#define NTP_SET_RECEIVE_SECS(NTP, V)    WRITE_4BYTE(NTP, 32, V)

#define NTP_GET_RECEIVE_FRACT(NTP)      READ_4BYTE(NTP, 36)
#define NTP_SET_RECEIVE_FRACT(NTP, V)   WRITE_4BYTE(NTP, 36, V)

#define NTP_GET_TRANSMIT_SECS(NTP)       READ_4BYTE(NTP, 40)
#define NTP_SET_TRANSMIT_SECS(NTP, V)    WRITE_4BYTE(NTP, 40, V)

#define NTP_GET_TRANSMIT_FRACT(NTP)      READ_4BYTE(NTP, 44)
#define NTP_SET_TRANSMIT_FRACT(NTP, V)   WRITE_4BYTE(NTP, 44, V)

#endif /* __INCLUDE_NTP_H */
