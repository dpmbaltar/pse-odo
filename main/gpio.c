
/*
 * driver gpio: permite operar bits de los puertos gpios individualmente,
 * o en forma paralela (con los 8 bits del puerto).
 */

#include "gpio.h"

/*
 *                               Mini-USB
 *                              [-|----|-]
 *           SCK PB5    D13  [13] |====| [12]  D12 PB4 MISO
 *                     3.3V  [  ] |____| [11] ~D11 PB3 MOSI OC2A
 *                     AREF  [  ]        [10] ~D10 PB2 SS   OC1B
 *               PC0 A0 D14  [14]        [ 9] ~D9  PB1      OC1A
 *               PC1 A1 D15  [15]        [ 8]  D8  PB0      CLK0/ICP1
 *               PC2 A2 D16  [16]        [ 7]  D7  PD7
 *               PC3 A3 D17  [17]        [ 6] ~D6  PD6      OC0A
 *           SDA PC4 A4 D18  [18] Arduino[ 5] ~D5  PD5      OC0B/T1
 *           SCL PC5 A5 D19  [19]  Nano  [ 4]  D4  PD4      T0/XCK
 *                   A6      [20]  V3.0  [ 3] ~D3  PD3      OC2B
 *                   A7      [21]        [ 2]  D2  PD2
 *                       5V  [  ]        [  ]  GND
 *                  PC6 RST  [  ]        [  ]  RST PC6 RST
 *                      GND  [  ]        [ 0]  D0  PD0 RX
 *                      VIN  [  ] [][][] [ 1]  D1  PD1 TX
 *                              [_[][][]_]
 *                                  ||
 *                                 _||_
 *                                 \  /
 *                                  \/
 *           SCK PB5    D13  -------+
 *                  PC6 RST  --[  ][13][12]--  D12 PB4 MISO
 *                      GND  --[  ][11][  ]--  5V
 *                                   +------- ~D11 PB3 MOSI OC2A
 *
 * Ejemplos de uso:
 *
 * // salida
 * gpio_output(13);
 * gpio_pin(13, ON);
 * gpio_pin(13, OFF);
 * gpio_pin(13, TOGGLE);
 *
 * // entrada
 * unsigned char v;
 * gpio_input(13);
 * v = gpio_pin(13, GET);
 *
 * // activar pull up
 * gpio_input(13);
 * gpio_pin(13, ON);
 *
 * // leer todo el registro de datos del puerto C (entrada)
 * gpio_input(PORT_C);
 * v = gpio_read(PORT_C);
 *
 * // escribir todo el registro de datos del puerto B (salida)
 * gpio_output(PORT_B);
 * v = 0x43;
 * gpio_write(PORT_B, v);
 */

/* gpio_pin(): opera sobre un pin individual (en salida, activar pull-up, o
 * leyendo su valor de entrada).
 *
 * Argumentos:
 * pin: etiqueta del pin en la placa (ejemplo: 13)
 * op es la operación: ON, OFF, TOGGLE o GET
 *
 */

volatile unsigned char *PINB = (unsigned char *)0x23;
volatile unsigned char *DDRB = (unsigned char *)0x24;
volatile unsigned char *PORTB = (unsigned char *)0x25;
volatile unsigned char *PINC = (unsigned char *)0x26;
volatile unsigned char *DDRC = (unsigned char *)0x27;
volatile unsigned char *PORTC = (unsigned char *)0x28;
volatile unsigned char *PIND = (unsigned char *)0x29;
volatile unsigned char *DDRD = (unsigned char *)0x2A;
volatile unsigned char *PORTD = (unsigned char *)0x2B;

int gpio_pin(int p, int op)
{
    unsigned char reg;
    unsigned char value;

    reg = p < 8 ? 0 : p < 14 ? 1 : 2;
    switch (reg) {
    case 0:
        if (op == 0)
            *PORTD &= ~(1 << p);
        else if (op == 1)
            *PORTD |= (1 << p);
        else if (op == 2)
            *PORTD ^= (1 << p);
        else
            value = (*PIND >> p) & 0x01;
        break;
    case 1:
        if (op == 0)
            *PORTB &= ~(1 << (p - 8));
        else if (op == 1)
            *PORTB |= (1 << (p - 8));
        else if (op == 2)
            *PORTB ^= (1 << (p - 8));
        else
            value = (*PINB >> (p - 8)) & 0x01;
        break;
    default:
        if (op == 0)
            *PORTC &= ~(1 << (p - 14));
        else if (op == 1)
            *PORTC |= (1 << (p - 14));
        else if (op == 2)
            *PORTC ^= (1 << (p - 14));
        else
            value = (*PINC >> (p - 14)) & 0x01;
        break;
    }

    return value;
}

/* establece el pin p (o puerto p) como entrada */
void gpio_input(int p)
{
    unsigned char reg;

    reg = p < 8 ? 0 : p < 14 ? 1
                             : 2;
    switch (reg) {
    case 0:
        *PORTD &= ~(1 << p);
        *DDRD &= ~(1 << p);
        break;
    case 1:
        *PORTB &= ~(1 << (p - 8));
        *DDRB &= ~(1 << (p - 8));
        break;
    default:
        *PORTC &= ~(1 << (p - 14));
        *DDRC &= ~(1 << (p - 14));
        break;
    }
}

/* establece el pin p (o puerto p) como salida */
void gpio_output(int p)
{
    unsigned char reg;

    reg = p < 8 ? 0 : p < 14 ? 1
                             : 2;
    switch (reg) {
    case 0:
        *PORTD &= ~(1 << p);
        *DDRD |= (1 << p);
        break;
    case 1:
        *PORTB &= ~(1 << (p - 8));
        *DDRB |= (1 << (p - 8));
        break;
    default:
        *PORTC &= ~(1 << (p - 14));
        *DDRC &= ~(1 << (p - 14));
        break;
    }
}

/* leer los 8 bits del puerto port */
unsigned char gpio_read(int PORT)
{
    volatile unsigned char *DIR_PIN;

    switch (PORT) {
    case PORT_B:
        DIR_PIN = (unsigned char *)0x23; // DIR_PINB
        break;

    case PORT_C:
        DIR_PIN = (unsigned char *)0x26; // DIR_PINC
        break;

    case PORT_D:
        DIR_PIN = (unsigned char *)0x29; // DIR_PIND
        break;

    default:
        break;
    }

    return *(DIR_PIN);
}

/* escribir los 8 bits del puerto port con el valor n */
inline void gpio_write(int PORT, unsigned char n)
{
    volatile unsigned char *DIR_PORT;

    switch (PORT) {
    case PORT_B:
        DIR_PORT = (unsigned char *)0x25; // DIR_PORTB
        break;

    case PORT_C:
        DIR_PORT = (unsigned char *)0x28; // DIR_PORTC
        break;

    case PORT_D:
        DIR_PORT = (unsigned char *)0x2B; // DIR_PORTD
        break;

    default:
        break;
    }

    if (PORT)
        *(DIR_PORT) = n;
}
