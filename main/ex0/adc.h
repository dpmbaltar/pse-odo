/**********************************************************************
 *
 * Filename:    adc.h
 * 
 * API para la aplicacion embebida 
 **********************************************************************/

#ifndef _ADC_H
#define _ADC_H

#define REF_5V 1
#define REF_1_1V 2
#define REF_EXTERNAL 3

// MULTIPLEXER SELECTION
#define REFS1 7
#define REFS0 6
#define ADLAR 5
#define MUX3 3
#define MUX2 2
#define MUX1 1
#define MUX0 0

void adc_init(void);
int adc_get(char channel, char ref);

#endif /* _SERIAL_H */
