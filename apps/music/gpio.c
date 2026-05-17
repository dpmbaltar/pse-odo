  
/*
 * driver gpio: permite operar bits de los puertos gpios individualmente,
 * o en forma paralela (con los 8 bits del puerto).
 */


/*
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

volatile unsigned char *PINB  = (unsigned char *) 0x23;
volatile unsigned char *DDRB  = (unsigned char *) 0x24;
volatile unsigned char *PORTB = (unsigned char *) 0x25;
volatile unsigned char *PINC  = (unsigned char *) 0x26;
volatile unsigned char *DDRC  = (unsigned char *) 0x27;
volatile unsigned char *PORTC = (unsigned char *) 0x28;
volatile unsigned char *PIND  = (unsigned char *) 0x29;
volatile unsigned char *DDRD  = (unsigned char *) 0x2A;
volatile unsigned char *PORTD = (unsigned char *) 0x2B; 

int gpio_pin(int p, int op) 
{
        unsigned char reg = p < 8? 0 : p < 14? 1 : 2;
        unsigned char value;
    
        switch(reg){
        case 0:
	        if (op == 0)
	                *PORTD &= ~(1 << p);
                else if (op == 1)
                        *PORTD |= (1<<p);
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
	unsigned char reg = p < 8? 0 : p < 14? 1 : 2;
    
        switch(reg){
        case 0:
	        *PORTD &= ~(1<<p); 
	        *DDRD &= ~(1<<p);
                break;
        case 1:
	        *PORTB &= ~(1<<(p-8)); 
	        *DDRB &= ~(1<<(p-8));
                break;
        default:
	        *PORTC &= ~(1<<(p-14)); 
	        *DDRC &= ~(1<<(p-14));
                break;
        }
}

/* establece el pin p (o puerto p) como salida */
void gpio_output(int p)
{
	unsigned char reg = p < 8? 0 : p < 14? 1 : 2;
    
        switch(reg){
        case 0:
	        *PORTD &= ~(1<<p); 
	        *DDRD |=  (1<<p); 
                break;
        case 1:
	        *PORTB &= ~(1<<(p-8)); 
	        *DDRB |=  (1<<(p-8)); 
                break;
        default:
	        *PORTC &= ~(1<<(p-14)); 
	        *DDRC &= ~(1<<(p-14));
                break;
        }
}

/* leer los 8 bits del puerto port */
void gpio_read(int port)
{
	/* COMPLETAR */
}

/* escribir los 8 bits del puerto port con el valor n */
void gpio_write(int port, unsigned char n)
{
	/* COMPLETAR */
}

