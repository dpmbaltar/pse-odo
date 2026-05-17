/*
 * tarea led: esta tarea parpadea un led cuando se activa un evento.
 */

#include <xinu.h>
#include "gpio.h"

extern sid32 evento1;

void led(int argc, const int * argv[])
{
	int pin = (int) argv[0];
	int frec = (int) argv[1];
	gpio_output(pin);

	while(1) {
		wait(evento1);
		gpio_pin(pin, ON);
		sleepms(frec);
		gpio_pin(pin, OFF);
		sleepms(frec);
	}
}
