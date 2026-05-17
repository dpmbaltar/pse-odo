/*
 * main_buffer: aplicacion embebida que utiliza un buffer sincronizado
 * 		para la comunicación entre dos tareas.
 */

#include <xinu.h>
#include "buffer.h"

void productor(void) 
{
	int i;

	for (i=0; i<9999; i++)
		buffer_put(i);
}

void consumidor(void) 
{
	int i,n;

	for (i=0; i<9999; i++) {
		n = buffer_get();
		serial_put_int(n, 4);
		serial_put_str("\n\r");
	}
}

void main(void) {
	buffer_init();
	serial_init();

	resume(create(productor, 256, 20, "pro", 0));
	resume(create(consumidor, 256, 20, "con", 0));
	for(;;);

}

