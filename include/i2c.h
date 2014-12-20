#ifndef __INCLUDE_I2C_H
#define __INCLUDE_I2C_H

/*
 * Slave read/write modes
 */
#define I2C_SLA_W           0x0
#define I2C_SLA_R           0x1

extern void
i2c_init(void);

extern void
i2c_slave_init(uint8_t address);

extern uint8_t
i2c_start(uint8_t address, uint8_t mode) ;

extern uint8_t
i2c_rep_start(uint8_t address, uint8_t mode) ;

extern uint8_t
i2c_send_byte(uint8_t data);

extern uint8_t
i2c_send_byte_nowait(uint8_t data, uint8_t ack);

extern uint8_t
i2c_tx_busy(void);

extern uint8_t
i2c_status(void);

extern uint8_t
i2c_read_byte_ack(void);

extern uint8_t
i2c_read_byte_nack(void);

extern void
i2c_stop(void);

#endif /* __INCLUDE_I2C_H */
