/*
 * ex0: ejemplo0. 
 * main es la primer tarea puesta en ejecución por XINU RTOS.
 * Esta tarea crea dos tareas y luego muestra un mensaje a 1hz.
 *
 * Las 3 tareas son gestionadas por el RTOS, quien las ejecuta
 * de manera concurrente. Ademas existe la tarea null, con menor prioridad.
 */

#include <xinu.h>
#include "serial.h"

void led();
void frec();


int main(void)
{
	int n = 0;

	serial_init();

	resume(create(led, 128, 20, "led", 2, 13, 500));
	resume(create(frec, 128, 20, "fre", 0));

	while(1) {
		sleepms(1000);

		n++;
		serial_put_str("hola \n\r");
		serial_put_int(n, 4);
		serial_put_str("\n\r");
	}
}


