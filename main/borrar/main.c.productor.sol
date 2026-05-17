/*
 * tarea 0: esta es la primer tarea puesta en ejecución por XINU RTOS.
 * Esta tarea crea dos tareas y luego muestra un mensaje a 1hz.
 *
 * Las 3 tareas son gestionadas por el RTOS, quien las ejecuta
 * de manera concurrente.
 */

#include <xinu.h>
#include "serial.h"

void led();
void frec();

sid32 prod;
sid32 cons;

int n;

void productor(void) {
	n = 0;
	int i;

	for (i=0; i<20; i++) {
		wait(prod);
		n++;
		signal(cons);
	}
}


void consumidor(void) {
	int i;

	for (i=0; i<20; i++) {
		wait(cons);
		serial_put_int(n, 4);
		serial_put_str("\r\n");
		signal(prod);
	}
}

int main(void)
{
	int n = 0;

	prod = semcreate(1);
	cons = semcreate(0);

	serial_init();

	resume(create(productor, 256, 20, "pro", 0));
	resume(create(consumidor, 256, 20, "con", 0));

	while(1) {
		sleepms(1000);

	}
}


