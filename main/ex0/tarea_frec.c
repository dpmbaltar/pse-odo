
/* tarea 2: esta tarea lee el potenciometro conectado en A0
 * y genera una señal cuadrada con cierta frecuencia a partir del valor leído.
 * (En clase se conectó un speaker para obtener un feedback de la señal de
 * salida.)
 */

#include <xinu.h>
#include "adc.h"
#include "gpio.h"

void frec() {
	int n;

	adc_init();
	gpio_output(4);

	while(1) {
		n = adc_get(MUX0, REF_5V);

		gpio_pin(4, ON);
		sleepms(n/4+10);
		gpio_pin(4, OFF);
		sleepms(n/4+10);
	}
}
