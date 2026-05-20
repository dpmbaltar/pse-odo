#include <stdint.h>
#include "adc.h"

/* ADCSRA - ADC Control and Status Register A */
#define ADPS0       0 /* Bit 0 - ADPS0: ADC Prescaler Select Bits */
#define ADPS1       1 /* Bit 1 - ADPS1: ADC Prescaler Select Bits */
#define ADPS2       2 /* Bit 2 - ADPS2: ADC Prescaler Select Bits */
#define ADIE        3 /* Bit 3 - ADIE: ADC Interrupt Enable */
#define ADIF        4 /* Bit 4 - ADIF: ADC Interrupt Flag */
#define ADATE       5 /* Bit 5 - ADATE: ADC Auto Trigger Enable */
#define ADSC        6 /* Bit 6 - ADSC: ADC Start Conversion */
#define ADEN        7 /* Bit 7 - ADEN: ADC Enable */

/* ADMUX - ADC Multiplexer Selection Register Bits */
#define MUX0        0 /* Bit 0 - MUX0: Analog Channel Selection Bits */
#define MUX1        1 /* Bit 1 - MUX1: Analog Channel Selection Bits */
#define MUX2        2 /* Bit 2 - MUX2: Analog Channel Selection Bits */
#define MUX3        3 /* Bit 3 - MUX3: Analog Channel Selection Bits */
#define RES0_       4 /* Bit 4 - Res: Reserved Bit */
#define ADLAR       5 /* Bit 5 - ADLAR: ADC Left Adjust Result */
#define REFS0       6 /* Bit 6 - REFS0: Reference Selection Bits */
#define REFS1       7 /* Bit 7 - REFS1: Reference Selection Bits */

/* ADC Prescaler Selections */
#define ADC_PS_2    (1 << ADPS0)
#define ADC_PS_4    (1 << ADPS1)
#define ADC_PS_8    (1 << ADPS1) | (1 << ADPS0)
#define ADC_PS_16   (1 << ADPS2)
#define ADC_PS_32   (1 << ADPS2) | (1 << ADPS0)
#define ADC_PS_64   (1 << ADPS2) | (1 << ADPS1)
#define ADC_PS_128  (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0)

/* Control */
#define ADC_ENABLE  (1 << ADEN)
#define ADC_CONVERT (1 << ADSC)


typedef struct
{
  uint8_t data_l;           /* ADCL - ADC Data Register Low Byte */
  uint8_t data_h;           /* ADCH - ADC Data Register High Byte */
  uint8_t control_status_a; /* ADCSRA - ADC Control and Status Register A */
  uint8_t control_status_b; /* ADCSRB - ADC Control and Status Register B */
  uint8_t multiplexer;      /* ADMUX - ADC Multiplexer Selection Register */
  uint8_t _reserved;
  uint8_t input_disable;    /* DIDR0 - Digital Input Disable Register 0 */

} adc_t;

static volatile adc_t *adc = (adc_t*)(0x78);

void adc_init(void)
{
  // Seleccionar AVcc = Aref con capacitor externo
  adc->multiplexer = (1 << REFS0);

  // Habilitar ADC
  // Establecer preescalador a 128 (16MHz/128 = 125KHz)
  adc->control_status_a = ADC_ENABLE | ADC_PS_128;
}

uint16_t adc_read(uint8_t channel)
{
  // Seleccionar canal ADC (sólos los bits 0:3)
  adc->multiplexer |= (channel & 0x0F);

  // Comenzar conversión
  adc->control_status_a |= ADC_CONVERT;

  // Esperar a que se complete la conversión
  while (adc->control_status_a & ADC_CONVERT);

  return *(uint16_t*)(adc);
}
