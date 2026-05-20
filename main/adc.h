/**
 * @file adc.h
 * @author Diego Pablo Matías Baltar <diego.baltar@est.fi.uncoma.edu.ar>
 * @brief Driver para ADC del atmega328p
 * @date 2024-05-19
 */

#ifndef _ADC_H
#define _ADC_H

#include <stdint.h>

/* Input Channel Selections */
#define ADC0   0x00
#define ADC1   0x01
#define ADC2   0x02
#define ADC3   0x03
#define ADC4   0x04
#define ADC5   0x05
#define ADC6   0x06
#define ADC7   0x07
#define ADC8   0x08 /* For temperature sensor */
/* 0x09:0x0D Reserved */
#define ADCVBG 0x0E
#define ADCGND 0x0F


void     adc_init(void);
uint16_t adc_read(uint8_t channel);


#endif /* _ADC_H */
