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
#include "pid.h"

/*-----------------------------------------------------------------------
 * Configuración de pines
 *-----------------------------------------------------------------------
 */
#define ENC1_A   2  /* PD2/INT0 */
#define ENC2_A   3  /* PD3/INT1 */
#define ENC1_B   4  /* PD4 */
#define ENC2_B   5  /* PD5 */
#define MOT1_PWM 9  /* PB1 */
#define MOT2_PWM 10 /* PB2 */
#define MOT1_A   14 /* PC0/A0 */
#define MOT1_B   15 /* PC1/A1 */
#define MOT2_A   16 /* PC2/A2 */
#define MOT2_B   17 /* PC3/A3 */

/*-----------------------------------------------------------------------
 * Configuración general
 *-----------------------------------------------------------------------
 */
#define PWM_MIN       70   /* PWM mínimo para el motor */
#define PWM_MAX       255  /* PWM máximo (100%) */
#define PWM_DEADZONE  50   /* Zona muerta del motor (TT amarillo 6V) */

#define ROTATION_TOL  2    /* Tolerancia de rotación */
#define ENCODER_CPR   600L /* Pasos por vuelta del encoder */

#define WHEEL_DIAM_MM 60L  /* Díametro de ruedas (mm) */
#define WHEEL_BASE_MM 135L /* Distancia entre centros de las ruedas (mm) */

/*-----------------------------------------------------------------------
 * Configuración PID
 *-----------------------------------------------------------------------
 * Las ganancias están expresadas en Q8:
 *     Kp = valor / 256
 * Valores iniciales (ajustar experimentalmente!)
 */
#define PID_MOTOR_KP      256     /* 1.0 */
#define PID_MOTOR_KI      0       /* 0.0625 */
#define PID_MOTOR_KD      0       /* ??? */

#define PID_MOTOR_I_LIMIT 1000
#define PID_MOTOR_LIMIT   PWM_MAX

#define PID_PERIOD_MS     20      /* Período de control encoders (ms) */

/*-----------------------------------------------------------------------
 * Direcciones del motor
 *-----------------------------------------------------------------------
 */
#define DIR_STOP         0 /* Parar/Reposo */
#define DIR_FORWARD      1 /* Adelante */
#define DIR_REVERSE      2 /* Reversa */
#define DIR_ROTATE_LEFT  3 /* Rotar a la izquierda */
#define DIR_ROTATE_RIGHT 4 /* Rotar a la derecha */

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
     * D2 = PD2 = INT0 / flanco ascendente
     * D3 = PD3 = INT1 / flanco ascendente
     */
    EICRA |= (1 << ISC01) | (1 << ISC00) | (1 << ISC11) | (1 << ISC10);
    EIMSK |= (1 << INT0) | (1 << INT1);

    sei();
}

static void set_direction(uint8_t dir)
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

/*
 * Sea D el díametro de las ruedas, B la distancia entre los centros:
 *
 *               angle * CPR * B
 * diferencial = ---------------
 *                   180 * D
 *
 * El diámetro de rueda se cancela en la ecuación si estamos usando
 * desplazamiento de arco de las ruedas? No: para el ángulo del chasis
 * necesitamos la relación B/R, por lo que en realidad:
 *
 *               angle * CPR * B
 * diferencial = ----------------
 *                   360 * r
 *
 * donde r = D/2
 */
static int16_t angle_to_encoder_delta(int16_t angle)
{
    int32_t numerator;
    int32_t denominator;

    numerator = (int32_t)angle * ENCODER_CPR * WHEEL_BASE_MM;
    denominator = 180L * WHEEL_DIAM_MM;

    return (int16_t)(numerator / denominator);
}

void motors_init()
{
    gpio_output(MOT1_A);
    gpio_output(MOT1_B);
    gpio_output(MOT2_A);
    gpio_output(MOT2_B);
}

void motor_task(uint8_t motor_n)
{
    volatile int16_t *encoder_count;
    hodor_ad_t motor_pwm_addr;
    hodor_ad_t motor_tpwm_addr;
    void (*motor_pwm_set)(uint8_t);
    if (motor_n == 1) {
        encoder_count = &left_encoder_count;
        motor_pwm_addr = LMOTOR_PWM;
        motor_tpwm_addr = LMOTOR_TPWM;
        motor_pwm_set = timer1_set_pwm_A;
    } else {
        encoder_count = &right_encoder_count;
        motor_pwm_addr = RMOTOR_PWM;
        motor_tpwm_addr = RMOTOR_TPWM;
        motor_pwm_set = timer1_set_pwm_B;
    }

    pid_t pid;

    int16_t target_speed;
    int16_t current_speed;

    int16_t target_pwm;
    int16_t current_encoder;
    int16_t previous_encoder;

    pid_init(&pid,
             PID_MOTOR_KP,
             PID_MOTOR_KI,
             PID_MOTOR_KD,
             PID_MOTOR_I_LIMIT,
             PID_MOTOR_LIMIT);

    cli();
    previous_encoder = *encoder_count;
    sei();

    set_direction(DIR_STOP);
    motor_pwm_set(0);

    while (1) {
        hodor_st_get(motor_tpwm_addr, &target_speed);

        cli();
        current_encoder = *encoder_count;
        sei();

        /*
         * Velocidad = incremento de posición desde la última muestra.
         * La unidad es:
         *     pasos / PID_PERIOD_MS
         */
        current_speed = current_encoder - previous_encoder;
        previous_encoder = current_encoder;

        if (target_speed > -PWM_DEADZONE && target_speed < PWM_DEADZONE) {
            target_speed = 0;

            /*
             * Evitar que el término integral quede acumulado mientras el motor
             * está detenido.
             */
            pid.integral = 0;
            pid.previous_error = 0;

            current_speed = 0;
            target_pwm = 0;

            set_direction(DIR_STOP);
            motor_pwm_set(0);
        } else {
            target_pwm = pid_update(
                &pid,
                target_speed,
                current_speed
            );

            // Determinar dirección
            if (target_pwm > 0) {
                set_direction(DIR_ROTATE_LEFT);
            } else if (target_pwm < 0) {
                set_direction(DIR_ROTATE_RIGHT);
                target_pwm = -target_pwm;
            } else {
                set_direction(DIR_STOP);
            }

            if (target_pwm > 0 && target_pwm < PWM_MIN) {
                target_pwm = PWM_MIN;
            }

            if (target_pwm > PWM_MAX) {
                target_pwm = PWM_MAX;
            }

            motor_pwm_set((uint8_t)target_pwm);
        }

        hodor_st_set(motor_pwm_addr, target_pwm);
        sleepms(PID_PERIOD_MS);
    }
}

/**
 * Tarea de lectura de datos del giróscopo mediante I2C.
 *
 * D18/A4/PC4 (SDA)
 * D19/A5/PC5 (SCL)
 */
void gyro_task(void)
{
    mpu6050_data_t imu;

    while (1) {
        mpu6050_read(&imu);
        hodor_st_set(GYRO_Z, imu.gz);
        sleepms(10);
    }
}

void battery_task(void)
{
    int16_t battery_mv = 0;

    while (1) {
        battery_mv = adc_read(ADC6);
        hodor_st_set(BATTERY_MV, battery_mv);
        sleepms(1000);
    }
}

void main(void)
{
    adc_init();
    serial_init();
    timer1_init();
    //twi_init();
    //mpu6050_init();
    hodor_init();
    motors_init();
    encoders_init();

    // Quitar la tarea encoder, y utilizar la misma tarea de motors_taks
    // con distintos parametros, enviar las velocidades desde linux y probar
    // la estructura y el paso de mensajes
    // probar con 2 valores uno bajo y otro alto
    resume(create(motor_task, 128, 20, "motor1", 1, 1));
    //resume(create(motor_task, 128, 20, "motor2", 1, 2));
    //resume(create(battery, 64, 20, "battery", 0));
    //resume(create(gyro_task, 192, 20, "gyro", 0));

    uint8_t op;
    uint8_t addr;
    int16_t value;
    hodor_msg_t msg = MSG_INIT;

    while (1) {
        if (hodor_msg_recv(&msg) == 0) {
            op = MSG_OP(msg.head);
            addr = MSG_ADDR(msg.head);
            switch (op) {
            case OP_READ:
                hodor_st_get((hodor_ad_t)addr, &value);
                msg.head = MSG_HEAD(OP_WRITE, (hodor_ad_t)addr);
                msg.body = value;
                hodor_msg_send(&msg);
                break;
            case OP_WRITE:
                hodor_st_set((hodor_ad_t)addr, msg.body);
                msg.head = MSG_HEAD(OP_ACK, (hodor_ad_t)addr);
                hodor_msg_send(&msg);
                break;
            case OP_ACK:
            default:
                msg.head = MSG_HEAD(OP_ACK, (hodor_ad_t)addr);
                msg.body = 0xffff;
                hodor_msg_send(&msg);
                break;
            }
        }
    }
}
