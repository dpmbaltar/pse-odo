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
#define ROTATION_TOL  2    /* Tolerancia de rotación */
#define ENCODER_CPR   600L /* Pasos por vuelta del encoder */
#define WHEEL_DIAM_MM 60L  /* Díametro de ruedas (mm) */
#define WHEEL_BASE_MM 135L /* Distancia entre centros de las ruedas (mm) */

#define DEADZONE      40   /* Zona muerta del motor (TT amarillo 6V) */
#define RAMP_STEP     1    /* Pasos de aceleración del motor */
#define RAMP_DELAY_MS 2    /* Uso en sleepms() de XINU para la tarea mot1 */

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

void encoders_task(void)
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

int16_t rotation_target = 90;

void motors_task(void)
{
    int16_t target;
    int16_t current;
    int16_t control;

    int16_t left_start;
    int16_t right_start;

    int16_t left;
    int16_t right;

    pid_t pid;

    pid_init(
        &pid,
        384,  /* Kp = 1.5 */
        8,    /* Ki = 0.03125 */
        128,  /* Kd = 0.5 */
        1000,
        100
    );

    hodor_st_get(LENC_STEPS, &left_start);
    hodor_st_get(LENC_STEPS, &right_start);
    target = angle_to_encoder_delta(rotation_target);

    while (1) {
        hodor_st_get(LENC_STEPS, &left);
        hodor_st_get(LENC_STEPS, &right);
        left  -= left_start;
        right -= right_start;
        current = right - left;

        /*
         * PID sobre el error angular.
         */
        control = pid_update(&pid, target, current);

        /*
         * Giro diferencial.
         */
        if (rotation_target < 0) {
            set_direction(DIR_ROTATE_LEFT);
        } else if (rotation_target > 0) {
            set_direction(DIR_ROTATE_RIGHT);
        } else {
            set_direction(DIR_STOP);
        }

        if (abs(target - current) <= ROTATION_TOL) {
            timer1_set_pwm_A(0);
            timer1_set_pwm_B(0);
            hodor_st_set(LMOTOR_TPWM, 0);
            hodor_st_set(RMOTOR_TPWM, 0);
        } else {
            timer1_set_pwm_A(PWM_MIN - control);
            timer1_set_pwm_B(PWM_MIN + control);
            hodor_st_set(LMOTOR_TPWM, PWM_MIN - control);
            hodor_st_set(RMOTOR_TPWM, PWM_MIN + control);
        }

        sleepms(20);
    }
}

void motor_task(void)
{
    int16_t target_pwm = 0;
    uint8_t current_pwm = 0;
    uint8_t target_dir = DIR_STOP;
    uint8_t current_dir = DIR_STOP;

    while (1) {
        hodor_st_get(LMOTOR_TPWM, &target_pwm);

        /*
         * Zona muerta
         */
        if (target_pwm > -DEADZONE && target_pwm < DEADZONE) {
            target_pwm = 0;
            target_dir = DIR_STOP;
        } else {
            /*
             * Determinar dirección
             */
            if (target_pwm > 0) {
                target_dir = DIR_ROTATE_LEFT;
            } else {
                target_dir = DIR_ROTATE_RIGHT;
                target_pwm = -target_pwm;
            }

            if (target_pwm > PWM_MAX)
                target_pwm = PWM_MAX;
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

                if (current_pwm > PWM_MAX)
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

        timer1_set_pwm_A(current_pwm);
        timer1_set_pwm_B(current_pwm);
        sleepms(RAMP_DELAY_MS);
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

    resume(create(motor_task, 128, 20, "motors", 1, 1));
    //resume(create(encoders_task, 128, 20, "encoders", 0));
    //Quitar la tarea encoder, y utilizar la misma tarea de motors_taks
    //con distintos parametros, enviar las velocidades desde linux y probar
    //la estructura y el paso de mensajes
    //probar con 2 valores uno bajo y otro alto
    //resume(create(battery, 64, 20, "battery", 0));
    //resume(create(gyro_task, 192, 20, "gyro", 0));

    uint8_t op;
    uint8_t addr;
    int16_t value;
    hodor_msg_t msg = MSG_INIT;

    while (1) {
        /*if (hodor_msg_recv(&msg) == 0) {
            op = MSG_OP(msg.head);
            addr = MSG_ADDR(msg.head);
            hodor_st_set((hodor_ad_t)addr, msg.body);

            msg.head = MSG_HEAD(OP_ACK, (hodor_ad_t)addr);
            hodor_msg_send(&msg);
        }*/

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
