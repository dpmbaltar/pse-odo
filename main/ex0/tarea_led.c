/*
 * tarea 1: esta tarea parpadea un led a cierta frecuencia.
 */

#include <xinu.h>
#include "gpio.h"


void led(int argc, const char * argv[])
{
	int pin = (int) argv[0];
	int frec = (int) argv[1];
	gpio_output(pin);

	while(1) {
		gpio_pin(pin, ON);
		sleepms(frec);
		gpio_pin(pin, OFF);
		sleepms(frec);
	}
}
