#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>
#include "gpio.h"

/** Valor absoluto */
#define ABS(x) ((x) < 0 ? (-x) : (x))

void melody_note(unsigned int frequency, double duration)
{

  double seconds = 1.0 / frequency / 2;
  double micros = seconds * 1000000L;
  double duration_us = duration * 1000L;

  gpio_output(5);
  for (long i = 0; i <= duration_us; i += micros * 2) {
	  gpio_pin(5, ON);
	  _delay_us(micros);
	  gpio_pin(5, OFF);
	  _delay_us(micros);
  }
}

