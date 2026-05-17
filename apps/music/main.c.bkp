#include <xinu.h>

void cancion();

int main(void)
{

	resume(create(cancion, 400, 20, "ca", 0));
	for(;;) {
		sleepms(1000);
		serial_put_str("hola\n\r");
	};
}

