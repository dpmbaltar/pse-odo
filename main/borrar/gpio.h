/*
 * driver gpio: permite operar bits de los puertos gpios individualmente,
 * o en forma paralela (con los 8 bits del puerto).
 */

#ifndef GPIO_H
#define GPIO_H

#define OFF 	0x0
#define ON 	0x1
#define TOGGLE	0x2
#define GET	0x3
#define PORT_B	0x4
#define PORT_C	0x5
#define PORT_D	0x6
#define A0 0x0E
#define A1 0x0F
#define A2 0x10
#define A3 0x11
#define A4 0x12
#define A5 0x13
#define A6 0x14
#define A7 0x15


int gpio_pin(int p, int op);
void gpio_input(int p);
void gpio_output(int p);
void gpio_read(int port);
void gpio_write(int port, unsigned char n);

#endif /* GPIO_H */
