/*
 * concurrencia solucionada: ejemplo de dos tareas con region critica, pero
 * 			     sincronizadas (protegias) por un mutex
 */

#include <xinu.h>
#include "mutex.h"

#define PROD 0

int n,r;

void produce_y_consume(void) 
{
	int i;

	for (;;) {
		mutex_lock(PROD);
		n++;
		if (n > (r+1))
			serial_put_str("error\n");
		r = n;
		mutex_unlock(PROD);
	}
}

void main(void) {
	mutex_init();
	serial_init();

	n = 0;
	r = n;
	resume(create(produce_y_consume, 256, 20, "pro", 0));
	resume(create(produce_y_consume, 256, 20, "pro", 0));

	for(;;) {
		serial_put_char('.');
	}
}

