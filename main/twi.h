#ifndef TWI_H
#define TWI_H

#include <stdint.h>

void twi_init(void);
void twi_start(uint8_t address);
void twi_stop(void);
uint8_t twi_write(uint8_t data);
uint8_t twi_read_ack(void);
uint8_t twi_read_nack(void);

#endif
