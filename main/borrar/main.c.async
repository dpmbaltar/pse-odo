/*
 * tarea 0: esta es la primer tarea puesta en ejecución por XINU RTOS.
 * Esta tarea crea dos tareas y luego muestra un mensaje a 1hz.
 *
 * Las 3 tareas son gestionadas por el RTOS, quien las ejecuta
 * de manera concurrente.
 */

#include <xinu.h>
#include "serial.h"

void led(int argc, char * argv[]);
void frec();

sid32 evento1;

int main(void)
{
	int n = 0;

	evento1 = semcreate(0);
	serial_init();

	resume(create(led, 128, 20, "led", 2, 12, 1000));
	resume(create(frec, 128, 20, "fre", 0));

	while(1) {
		sleep(5);
		signal(evento1);

		n++;
		serial_put_str("hola \n\r");
		serial_put_int(n, 4);
		serial_put_str("\n\r");
	}
}


