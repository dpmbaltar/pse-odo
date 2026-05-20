/**********************************************************************
 *
 * serial.c - Driver del UART del atmega328p
 *
 * META : ocultar el hardware a la aplicacion
 *
 * Configuracion: 9600bps, 8bits data, 1bit stop, sin bit de paridad
 *
 **********************************************************************/

#include <stdint.h>

/* UCSR0A - USART Control and Status Register A bits */
#define UCSR0A_MPCM0   0
#define UCSR0A_U2X0    1
#define UCSR0A_UPE0    2
#define UCSR0A_DOR0    3
#define UCSR0A_FE0     4
#define UCSR0A_UDRE0   5
#define UCSR0A_TXC0    6
#define UCSR0A_RXC0    7

/* UCSR0B - USART Control and Status Register B bits */
#define UCSR0B_TXB80   0
#define UCSR0B_RXB80   1
#define UCSR0B_UCSZ02  2
#define UCSR0B_TXEN0   3
#define UCSR0B_RXEN0   4
#define UCSR0B_UDRIE0  5
#define UCSR0B_TXCIE0  6
#define UCSR0B_RXCIE0  7

/* UCSR0C - USART Control and Status Register C bits */
#define UCSR0C_UCPOL0  0
#define UCSR0C_UCSZ00  1
#define UCSR0C_UCSZ01  2
#define UCSR0C_USBS0   3
#define UCSR0C_UPM00   4
#define UCSR0C_UPM01   5
#define UCSR0C_UMSEL00 6
#define UCSR0C_UMSEL01 7

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE  (((F_CPU/(USART_BAUDRATE*16UL)))-1)

#define RX_ENABLE      (1 << UCSR0B_RXEN0)  /* Habilitar la recepción */
#define TX_ENABLE      (1 << UCSR0B_TXEN0)  /* Habilitar la transmisión */
#define PARITY_MODE    (0 << UCSR0C_UPM00)  /* No usar bit de paridad */
#define STOP_BITS      (0 << UCSR0C_USBS0)  /* Usar 1 bit de stop */
#define CHAR_SIZE      (3 << UCSR0C_UCSZ00) /* Usar 8 bits de dato de E/S */
#define READY_TO_READ  (1 << UCSR0A_RXC0)   /* Dato listo para leer */
#define READY_TO_WRITE (1 << UCSR0A_UDRE0)  /* Búfer listo para escribir */

/* Máximo entre dos números */
#define MAX(a,b) ((a) < (b) ? (b) : (a))

/* Mínimo entre dos números */
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define MAX_INT_DIGITS       5
#define MAX_LONG_DIGITS      10
#define MAX_DOUBLE_PRECISION 10

/* Estructura de datos del USART del atmega328, según sugerido en el apunte */
typedef struct
{
  uint8_t status_control_a; /* ucsr0a USART Control and Status A */
  uint8_t status_control_b; /* ucsr0b USART Control and Status B */
  uint8_t status_control_c; /* ucsr0c USART Control and Status C */

  uint8_t _reserved;        /* reserved */

  uint8_t baud_rate_l;      /* ubrr0l baud rate low */;
  uint8_t baud_rate_h;      /* ubrr0h baud rate high */

  uint8_t data_io;          /* udr0 I/O data */

} volatile uart_t;

/* Puntero a la estructura de los registros del periférico */
uart_t *serial_port = (uart_t*)(0xc0);

void serial_init()
{
  // Establecer baud rate
  serial_port->baud_rate_h = (uint8_t)(BAUD_PRESCALE >> 8);
  serial_port->baud_rate_l = (uint8_t)(BAUD_PRESCALE);

  // Habilitar recepción y transmisión
  serial_port->status_control_b = RX_ENABLE | TX_ENABLE;

  // Establecer formato del frame
  serial_port->status_control_c = CHAR_SIZE | STOP_BITS | PARITY_MODE;
}

/* Enviar un byte a través del del dispositivo inicializado */
void serial_put_char(char data)
{
  // Se debe esperar verificando el bit UDREn del registro UCSRnA
  // hasta que el búfer esté listo para recibir un dato a transmitir
  while (!(serial_port->status_control_a & READY_TO_WRITE));

  // Colocar dato en el búfer para ser enviado
  serial_port->data_io = data;
}

/* Recibir un byte a través del del dispositivo inicializado */
char serial_get_char(void)
{
  // Esperar para recibir el dato
  while (!(serial_port->status_control_a & READY_TO_READ));

  // Devolver el valor que se encuentra en el registro de datos de E/S
  return serial_port->data_io;
}

void serial_put_str(char *str)
{
  char *c = str;

  while (*c != '\0') {
    serial_put_char(*c);
    c++;
  }
}

void serial_get_str(char *str, int len)
{
  char c = 0;
  char n = 0;

  while (c != '\n' && c != '\r' && n < len) {
    c = serial_get_char();
    *(str+n) = c;
    n++;
  }
}

void serial_put_long_int(long int value, int num_digits)
{
  char buffer[MAX_LONG_DIGITS] = {0};
  int i = 0;

  if (value == 0) {
    return serial_put_char('0');
  } else if (value < 0) {
    serial_put_char('-');
    value = -value; // Usar valor absoluto
  }

  while (value != 0) {
    buffer[i] = value % 10;
    value /= 10;
    i++;
  }

  if (num_digits < 1 || num_digits > MAX_LONG_DIGITS) {
    num_digits = i;
  }

  for (int j = num_digits - 1; j > -1; j--) {
    serial_put_char(48 + buffer[j]);
  }
}

void serial_put_int(int value, int num_digits)
{
  if (num_digits > MAX_INT_DIGITS) {
    num_digits = MAX_INT_DIGITS;
  }

  return serial_put_long_int(value, num_digits);
}

void serial_put_double(double value, int int_digits, int frac_digits)
{
  long int int_value = (long int)value;
  double frac_value = value - int_value;
  int frac_digit = 0;

  serial_put_long_int(int_value, int_digits);
  serial_put_char('.');

  if (frac_digits > MAX_DOUBLE_PRECISION) {
    frac_digits = MAX_DOUBLE_PRECISION;
  } else if (frac_digits < 1) {
    frac_digits = 1;
  }

  if (frac_value < 0) {
    frac_value = -frac_value;
  }

  for (int i = 0; i < frac_digits; i++) {
    frac_value *= 10;
    frac_digit = (int)frac_value;
    serial_put_char(48 + (char)frac_value);
    frac_value -= frac_digit;
  }
}
