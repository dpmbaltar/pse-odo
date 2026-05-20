/**
 * @file serial.h
 * @author Diego Pablo Matías Baltar <diego.baltar@est.fi.uncoma.edu.ar>
 * @brief Driver del UART del atmega328p
 * @date 2024-05-01
 *
 * API para la aplicacion embebida.
 * META: ocultar el hardware a la aplicacion.
 */

#ifndef _SERIAL_H
#define _SERIAL_H


void serial_init(void);
void serial_put_char(char c);
void serial_put_str(char *s);
void serial_put_int(int value, int num_digits);
void serial_put_long_int(long int value, int num_digits);
void serial_put_double(double value, int int_digits, int frac_digits);
char serial_get_char(void);
void serial_get_str(char *str, int len);


#endif /* _SERIAL_H */
