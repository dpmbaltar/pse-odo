/**********************************************************************
 *
 * adc.c - Driver del ADC del atmega328p
 *
 **********************************************************************/

#include "adc.h"
#include <stdint.h> /* para los tipos de datos. Ej.: uint8_t */

#define CHANNEL_MASK 0b00000111

#define REF_5V 1
#define REF_1_1V 2
#define REF_EXTERNAL 3

// CONTROL AND STATUS
#define ADEN 7
#define ADSC 6
#define ADATE 5
#define ADIF 4
#define ADIE 3
#define ADPS2 2
#define ADPS1 1
#define ADPS0 0

// MULTIPLEXER SELECTION
#define REFS1 7
#define REFS0 6
#define ADLAR 5
#define MUX3 3
#define MUX2 2
#define MUX1 1
#define MUX0 0

// CONFIGS
#define ENABLE (1 << ADEN)
#define PRESCALER_128 (0b111 << ADPS0)
#define START_CONVERSION (1 << ADSC)
#define EXTERNAL_REF (0b00 << REFS0)
#define INTERNAL_VCC (0b01 << REFS0)
#define INTERNAL_1_1V (0b11 << REFS0)

typedef struct
{
  uint8_t data_l;
  uint8_t data_h;
  uint8_t control_status_a;
  uint8_t control_status_b;
  uint8_t multiplexer_selection;
  uint8_t reserved;
  uint8_t digital_input_disable_0;
  uint8_t digital_input_disable_1;
} volatile adc_t;

adc_t *adc = (adc_t *)0x78;

void
adc_init ()
{
  adc->control_status_a = ENABLE | PRESCALER_128;
}

int
adc_get (char channel, char ref)
{
  int result;

  switch (ref)
    {
    case REF_EXTERNAL:
      adc->multiplexer_selection = EXTERNAL_REF;
      break;
    case REF_1_1V:
      adc->multiplexer_selection = INTERNAL_1_1V;
    default:
      adc->multiplexer_selection = INTERNAL_VCC;
      break;
    }

  adc->multiplexer_selection |= (channel & CHANNEL_MASK);
  adc->control_status_a |= START_CONVERSION;
  while ((adc->control_status_a) & (1 << START_CONVERSION))
    ;
  result = adc->data_l;
  result |= (adc->data_h << 8);
  return result;
}