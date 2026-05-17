/*
 * concurrencia con problemas: ejemplo de dos tareas con condicion de carrera
 * 			       (region crítica)
 */

#include <xinu.h>

int n,r;

void produce_y_consume(void) 
{
	int i;

	for (;;) {
		n++;
		if (n > (r+1))
			serial_put_str("error\n");
		r = n;
	}
}

void main(void) {
	serial_init();

	n = 0;
	r = n;
	resume(create(produce_y_consume, 256, 20, "pro", 0));
	resume(create(produce_y_consume, 256, 20, "pro", 0));

	for(;;) {
		sleepms(400);
		serial_put_char('.');
	}
}

