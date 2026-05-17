/*
 * ex1: ejemplo 1. dos tareas productor-consumidor. 
 * No están sincronizadas.
 *
 * Las 3 tareas son gestionadas por el RTOS (main es una tarea), quien 
 * las ejecuta de manera concurrente.
 * Además existe la cuarta tarea, null, de menor prioridad.
 */

#include <xinu.h>
#include "serial.h"

int n;

void productor(void) {
	n = 0;
	int i;

	for (i=0; i<20; i++)
		n++;
}


void consumidor(void) {
	int i;

	for (i=0; i<20; i++) {
		serial_put_int(n, 4);
		serial_put_str("\r\n");
	}
}

int main(void)
{
	int n = 0;

	serial_init();

	resume(create(productor, 256, 20, "pro", 0));
	resume(create(consumidor, 256, 20, "con", 0));

	/* esta tarea solo duerme */
	while(1) {
		sleepms(1000);

	}
}


