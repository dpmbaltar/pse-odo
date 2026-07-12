#include "twi.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define SCL_CLOCK 100000UL

/* TWBR – TWI Bit Rate Register bits */
#define TWBR7 7 /* Bits 7:0 – TWI Bit Rate Register */
#define TWBR6 6
#define TWBR5 5
#define TWBR4 4
#define TWBR3 3
#define TWBR2 2
#define TWBR1 1
#define TWBR0 0

/* TWSR – TWI Status Register bits */
#define TWS7  7 /* Bits 7:3 – TWS: TWI Status */
#define TWS6  6
#define TWS5  5
#define TWS4  4
#define TWS3  3
#define RES2_ 2 /* Bit 2 – Res: Reserved Bit */
#define TWPS1 1 /* Bits 1:0 – TWPS: TWI Prescaler Bits */
#define TWPS0 0

/* TWAR – TWI (Slave) Address Register bits */
#define TWA6  7 /* Bits 7:1 – TWA: TWI (Slave) Address Register */
#define TWA5  6
#define TWA4  5
#define TWA3  4
#define TWA2  3
#define TWA1  2
#define TWA0  1
#define TWGCE 0 /* Bit 0 – TWGCE: TWI General Call Recognition Enable Bit */

/* TWDR – TWI Data Register bits */
#define TWD7 7
#define TWD6 6
#define TWD5 5
#define TWD4 4
#define TWD3 3
#define TWD2 2
#define TWD1 1
#define TWD0 0

/* TWCR – TWI Control Register bits */
#define TWINT 7 /* Bit 7 – TWINT: TWI Interrupt Flag */
#define TWEA  6 /* Bit 6 – TWEA: TWI Enable Acknowledge Bit */
#define TWSTA 5 /* Bit 5 – TWSTA: TWI START Condition Bit */
#define TWSTO 4 /* Bit 4 – TWSTO: TWI STOP Condition Bit */
#define TWWC  3 /* Bit 3 – TWWC: TWI Write Collision Flag */
#define TWEN  2 /* Bit 2 – TWEN: TWI Enable Bit */
#define RES1_ 1 /* Bit 1 – Res: Reserved Bit */
#define TWIE  0 /* Bit 0 – TWIE: TWI Interrupt Enable */

/* TWAMR – TWI (Slave) Address Mask Register bits */
#define TWAM7 7 /* Bits 7:1 – TWAM: TWI Address Mask */
#define TWAM5 5
#define TWAM4 4
#define TWAM3 3
#define TWAM2 2
#define TWAM1 1
#define TWAM0 0 /* Bit 0 – Res: Reserved Bit */

/* 2-wire interface */
typedef struct __attribute__((packed))
{
    uint8_t twbr;  /* TWBR – TWI Bit Rate Register */
    uint8_t twsr;  /* TWSR – TWI Status Register */
    uint8_t twar;  /* TWAR – TWI (Slave) Address Register */
    uint8_t twdr;  /* TWDR – TWI Data Register */
    uint8_t twcr;  /* TWCR – TWI Control Register */
    uint8_t twamr; /* TWAMR - TWI (Slave) Address Mask Register */

} twi_t;

static volatile twi_t *const twi = (volatile twi_t *)0xB8;

void twi_init(void)
{
    twi->twsr = 0;
    twi->twbr = ((F_CPU / SCL_CLOCK) - 16) / 2;
    twi->twcr = (1 << TWEN);
}

void twi_start(uint8_t address)
{
    twi->twcr = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    while (!(twi->twcr & (1 << TWINT)))
        ;

    twi->twdr = address;
    twi->twcr = (1 << TWINT) | (1 << TWEN);

    while (!(twi->twcr & (1 << TWINT)))
        ;
}

void twi_stop(void)
{
    twi->twcr = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

    while (twi->twcr & (1 << TWSTO))
        ;
}

uint8_t twi_write(uint8_t data)
{
    twi->twdr = data;
    twi->twcr = (1 << TWINT) | (1 << TWEN);

    while (!(twi->twcr & (1 << TWINT)))
        ;

    return 0;
}

uint8_t twi_read_ack(void)
{
    twi->twcr = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);

    while (!(twi->twcr & (1 << TWINT)))
        ;

    return twi->twdr;
}

uint8_t twi_read_nack(void)
{
    twi->twcr = (1 << TWINT) | (1 << TWEN);

    while (!(twi->twcr & (1 << TWINT)))
        ;

    return twi->twdr;
}

