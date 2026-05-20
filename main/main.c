#include <avr/io.h>
#include <avr/interrupt.h>
#include <xinu.h>

#include "adc.h"
#include "serial.h"
#include "timer.h"

/*-----------------------------------------------------------------------
 * Configuración del motor (TT amarillo 6V)
 *-----------------------------------------------------------------------
 */
#define DEADZONE 40  /* Zona muerta del motor */
#define MIN_PWM  70  /* Mínimo PWM para que gire */
#define MAX_PWM  255 /* PWM máximo (100%) */

#define RAMP_STEP 1
#define RAMP_DELAY_MS 2

/*-----------------------------------------------------------------------
 * Direcciones para el motor
 *-----------------------------------------------------------------------
 */
#define DIR_STOP    0
#define DIR_FORWARD 1
#define DIR_REVERSE 2

/*-----------------------------------------------------------------------
 * Encoder
 *-----------------------------------------------------------------------
 */

/*
 * D2 -> Encoder A -> INT0
 * D3 -> Encoder B
 */
volatile int32_t encoder_count = 0;

/*
 * Interrupción por flanco ascendente en canal A
 */
ISR(INT0_vect)
{
    /*
     * Leer B (PD3)
     */
    if (PIND & (1 << PD3)) {
        encoder_count--;
    } else {
        encoder_count++;
    }
}

void enc1_init(void)
{
    /*
     * D2 y D3 como entradas
     */
    DDRD &= ~(1 << PD2);
    DDRD &= ~(1 << PD3);

    /*
     * Pull-ups internas (quitar si el encoder ya tiene pull-up externas)
     */
    //PORTD |= (1 << PD2);
    //PORTD |= (1 << PD3);

    /*
     * INT0 flanco ascendente
     */
    EICRA |= (1 << ISC01);
    EICRA |= (1 << ISC00);
    EIMSK |= (1 << INT0);

    sei();
}

void enc1(void)
{
    int32_t pos;

    while (1) {
        cli();
        pos = encoder_count;
        sei();

        serial_put_str("ENC1: ", 4);
        serial_put_long_int(pos, 0);
        serial_put_str("\r\n");
        sleepms(400);
    }
}

void set_direction(uint8_t dir)
{
    switch (dir) {
    case DIR_FORWARD:
        PORTD |= (1 << PD7);
        PORTB &= ~(1 << PB0);
        break;

    case DIR_REVERSE:
        PORTD &= ~(1 << PD7);
        PORTB |= (1 << PB0);
        break;

    default:
        PORTD &= ~(1 << PD7);
        PORTB &= ~(1 << PB0);
        break;
    }
}

void mot1_init()
{
    /*
     * D7 y D8 como salida
     */
    DDRD |= (1 << PD7);
    DDRB |= (1 << PB0);
}

void mot1(void)
{
    uint16_t adc_value;
    int16_t error;
    uint8_t target_pwm = 0;
    uint8_t current_pwm = 0;
    uint8_t target_dir = DIR_STOP;
    uint8_t current_dir = DIR_STOP;
    uint32_t temp;

    while (1) {
        adc_value = adc_read(PC0);
        error = (int16_t)adc_value - 512;

        /*
         * Zona muerta
         */
        if (error > -DEADZONE && error < DEADZONE) {
            target_pwm = 0;
            target_dir = DIR_STOP;
        } else {
            /*
             * Determinar dirección
             */
            if (error > 0) {
                target_dir = DIR_FORWARD;
            } else {
                target_dir = DIR_REVERSE;
                error = -error;
            }

            /*
             * Mapear:
             * DEADZONE..511 -> MIN_PWM..255
             */
            temp = (uint32_t)(error - DEADZONE) * (MAX_PWM - MIN_PWM);
            temp /= (511 - DEADZONE);
            target_pwm = MIN_PWM + temp;

            if (target_pwm > MAX_PWM)
                target_pwm = MAX_PWM;
        }

        /*
         * Cambio de dirección seguro
         */
        if (current_dir != target_dir) {
            /*
             * Frenar antes de cambiar de dirección
             */
            if (current_pwm > 0) {
                current_pwm -= RAMP_STEP;

                if (current_pwm > 255)
                    current_pwm = 0;
            } else {
                current_dir = target_dir;
                set_direction(current_dir);
            }
        } else {
            /*
             * Rampa de aceleración normal
             */
            if (current_pwm < target_pwm) {
                current_pwm += RAMP_STEP;

                if (current_pwm > target_pwm)
                    current_pwm = target_pwm;
            } else if (current_pwm > target_pwm) {
                current_pwm -= RAMP_STEP;

                if (current_pwm < target_pwm)
                    current_pwm = target_pwm;
            }
        }

        timer1_pulse(current_pwm);
        sleepms(RAMP_DELAY_MS);
    }
}

void giro(void)
{
    while (1) {
        serial_put_str("GIR", 4);
        serial_put_str("\r\n");
        sleepms(300);
    }
}

void bate(void)
{
    while (1) {
        serial_put_str("BAT", 4);
        serial_put_str("\r\n");
        sleepms(500);
    }
}

void main(void)
{
    adc_init();
    serial_init();
    timer1_init(0);

    mot1_init();
    enc1_init();

    resume(create(mot1, 128, 20, "mot1", 0));
    resume(create(enc1, 256, 20, "enc1", 0));
    //resume(create(bate, 128, 20, "bate", 0));
    //resume(create(giro, 128, 20, "giro", 0));

    for (;;)
        ;
}
