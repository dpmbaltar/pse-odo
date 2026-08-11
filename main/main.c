#include <avr/io.h>
#include <avr/interrupt.h>

#include <xinu.h>

#include "adc.h"
#include "serial.h"
#include "timer.h"
#include "gpio.h"
#include "twi.h"
#include "mpu6050.h"
#include "hodor.h"

/*-----------------------------------------------------------------------
 * Configuración de pines para encoders y motores (XY-160D)
 *-----------------------------------------------------------------------
 */
#define ENC1_A 2 /* PD2/INT0 */
#define ENC1_B 4 /* PD4 */

#define ENC2_A 3 /* PD3/INT1 */
#define ENC2_B 5 /* PD5 */

#define MOT1_PWM 9  /* PB1 */
#define MOT1_A   14 /* PC0/A0 */
#define MOT1_B   15 /* PC1/A1 */

#define MOT2_PWM 10 /* PB2 */
#define MOT2_A   16 /* PC2/A2 */
#define MOT2_B   17 /* PC3/A3 */

/*-----------------------------------------------------------------------
 * Configuración del motor (TT amarillo 6V)
 *-----------------------------------------------------------------------
 */
#define DEADZONE 40  /* Zona muerta del motor */
#define MIN_PWM  70  /* Mínimo PWM para que gire */
#define MAX_PWM  255 /* PWM máximo (100%) */

#define RAMP_STEP     1 /* Pasos de aceleración del motor */
#define RAMP_DELAY_MS 2 /* Uso en sleepms() de XINU para la tarea mot1 */

/*-----------------------------------------------------------------------
 * Direcciones del motor
 *-----------------------------------------------------------------------
 */
#define DIR_STOP         0
#define DIR_FORWARD      1
#define DIR_REVERSE      2
#define DIR_ROTATE_LEFT  3
#define DIR_ROTATE_RIGHT 4

/*-----------------------------------------------------------------------
 * Encoder
 *-----------------------------------------------------------------------
 */

/*
 * D2 -> Encoder1 A -> INT0
 * D3 -> Encoder2 A -> INT1
 */
volatile int16_t left_encoder_count = 0;
volatile int16_t right_encoder_count = 0;

/*
 * Interrupción por flanco ascendente en canal A
 */
ISR(INT0_vect)
{
    if (gpio_pin(ENC1_B, GET)) {
        left_encoder_count--;
    } else {
        left_encoder_count++;
    }
}

ISR(INT1_vect)
{
    if (gpio_pin(ENC2_B, GET)) {
        right_encoder_count--;
    } else {
        right_encoder_count++;
    }
}

void encoders_init(void)
{
    gpio_input(ENC1_A);
    gpio_input(ENC1_B);
    gpio_input(ENC2_A);
    gpio_input(ENC2_B);

    /*
     * Pull-ups internas (quitar si el encoder ya tiene pull-up externas)
     */
    gpio_pin(ENC1_A, ON);
    gpio_pin(ENC1_B, ON);
    gpio_pin(ENC2_A, ON);
    gpio_pin(ENC2_B, ON);

    /**
     * D2 = PD2 = INT0 flanco ascendente
     * D3 = PD3 = INT1 flanco ascendente
     */
    EICRA |= (1 << ISC01) | (1 << ISC00) | (1 << ISC11) | (1 << ISC10);
    EIMSK |= (1 << INT0) | (1 << INT1);

    sei();
}

void encoders(void)
{
    int16_t left_steps, right_steps;

    while (1) {
        cli();
        left_steps = left_encoder_count;
        right_steps = right_encoder_count;
        sei();

        hodor_st_set(LENC_STEPS, left_steps);
        hodor_st_set(RENC_STEPS, right_steps);
        sleepms(20);//probar cada 10-20ms
    }
}

void set_direction(uint8_t dir)
{
    switch (dir) {
    case DIR_FORWARD:
        gpio_pin(MOT1_A, ON);
        gpio_pin(MOT1_B, OFF);
        gpio_pin(MOT2_A, ON);
        gpio_pin(MOT2_B, OFF);
        break;

    case DIR_REVERSE:
        gpio_pin(MOT1_A, OFF);
        gpio_pin(MOT1_B, ON);
        gpio_pin(MOT2_A, OFF);
        gpio_pin(MOT2_B, ON);
        break;

    case DIR_ROTATE_LEFT:
        gpio_pin(MOT1_A, ON);
        gpio_pin(MOT1_B, OFF);
        gpio_pin(MOT2_A, OFF);
        gpio_pin(MOT2_B, ON);
        break;

    case DIR_ROTATE_RIGHT:
        gpio_pin(MOT1_A, OFF);
        gpio_pin(MOT1_B, ON);
        gpio_pin(MOT2_A, ON);
        gpio_pin(MOT2_B, OFF);
        break;

    default:
        gpio_pin(MOT1_A, OFF);
        gpio_pin(MOT1_B, OFF);
        gpio_pin(MOT2_A, OFF);
        gpio_pin(MOT2_B, OFF);
        break;
    }
}

void motors_init()
{
    gpio_output(MOT1_A);
    gpio_output(MOT1_B);
    gpio_output(MOT2_A);
    gpio_output(MOT2_B);
}

void motors(void)
{
    uint16_t adc_value;
    int16_t error;
    uint8_t target_pwm = 0;
    uint8_t current_pwm = 0;
    uint8_t target_dir = DIR_STOP;
    uint8_t current_dir = DIR_STOP;
    uint32_t temp;

    while (1) {
        adc_value = adc_read(ADC7);
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
                target_dir = DIR_ROTATE_LEFT;
            } else {
                target_dir = DIR_ROTATE_RIGHT;
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

/**
 * Tarea de lectura de datos del giróscopo mediante I2C.
 *
 * D18/A4/PC4 (SDA)
 * D19/A5/PC5 (SCL)
 */
void gyro(void)
{
    mpu6050_data_t imu;

    while (1) {
        mpu6050_read(&imu);
        hodor_st_set(GYRO_Z, imu.gz);
        sleepms(10);
    }
}

void bate(void)
{
    int16_t battery_mv = 0;

    while (1) {
        battery_mv = adc_read(ADC6);
        hodor_set(BATTERY_MV, battery_mv);
        sleepms(1000);
    }
}

void main(void)
{
    adc_init();
    serial_init();
    timer1_init(0);
    //twi_init();
    //mpu6050_init();
    hodor_init();
    motors_init();
    encoders_init();

    resume(create(motors, 128, 20, "motors", 0));
    resume(create(encoders, 128, 20, "encoders", 0));
    //resume(create(bate, 64, 20, "bate", 0));
    //resume(create(gyro, 128, 20, "gyro", 0));

    /*
    int16_t lpwm, rpwm;
    int16_t lsteps, rsteps;
    hodor_msg_t msg = MSG_INIT;
    */

    while (1) {
        /*
        hodor_st_get(LMOTOR_PWM, &lpwm);
        msg.head = MSG_HEAD(OP_WRITE, LMOTOR_PWM);
        msg.lmotor_pwm = lpwm;
        hodor_msg_send(&msg);

        hodor_st_get(RMOTOR_PWM, &rpwm);
        msg.head = MSG_HEAD(OP_WRITE, RMOTOR_PWM);
        msg.rmotor_pwm = rpwm;
        hodor_msg_send(&msg);
        */

        sleepms(500);
    }
}
