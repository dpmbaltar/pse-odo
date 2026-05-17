/*
 * ex3: ejemplo3, donde una tarea es bloqueada por el RTOS, hasta que suceda
 * el evento. Como la tarea bloqueada tiene mayor prioridad que las demás
 * tareas, XINU le asigna la CPU al momento de que suceda el evento y 
 * desbloquea.
 *
 * Las 3 tareas son gestionadas por el RTOS. main, null y led.
 */

#include <xinu.h>
#include <stdlib.h>
#include "serial.h"

void led(int argc, char * argv[]);

sid32 evento1;

int main(void)
{
	int n = 0;

	evento1 = semcreate(0);
	serial_init();

	resume(create(led, 128, 40, "led", 2, 12, 1000));

	while(1) {
		n = rand() % 10;
		sleep(n);
		signal(evento1);

		serial_put_str("Se activa evento\n\r");
	}
}


