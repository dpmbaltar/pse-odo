#ifndef HODOR_H
#define HODOR_H

#include <xinu.h>

#define SOF_RECV 0x55 /**< SOF: Arduino <= Linux */
#define SOF_SEND 0xAA /**< SOF: Arduino => Linux */

#define AD_MASK  0xFC /**< Máscara del offset del byte del estado */
#define OP_MASK  0x03 /**< Máscara de la operación sobre el estado */

#define OP_READ  1    /**< Operación de lectura */
#define OP_WRITE 2    /**< Operación de escritura */

#define MSG_INIT {0, 0, 0, 0}
#define MSG_HEAD(op, ad) (((op) & OP_MASK) | ((ad) & AD_MASK))

typedef struct __attribute__((packed))
{
    uint8_t sof;             /**< Start Of Frame */

    int16_t l_encoder_steps; /**< Pasos del encoder izquierdo */
    int16_t r_encoder_steps; /**< Pasos del encoder derecho */

    int16_t l_motor_pwm;     /**< PWM del motor izquierdo */
    int16_t r_motor_pwm;     /**< PWM del motor derecho */

    //int16_t l_motor_target_pwm;
    //int16_t r_motor_target_pwm;

    uint16_t battery_mv;     /**< Estado de batería */

    int16_t gyro_z; // Enviar a 100hz

    uint8_t checksum;

} hodor_st_t;

/** Offsets para acceder al estado del HODOR */
typedef enum
{
    LENC_STEPS = 1,  /**< Pasos del encoder izquierdo */
    RENC_STEPS = 3,  /**< Pasos del encoder derecho */
    LMOTOR_PWM = 5,  /**< PWM del motor izquierdo */
    RMOTOR_PWM = 7,  /**< PWM del motor derecho */
    BATTERY_MV = 9,  /**< Estado de batería */
    GYRO_Z     = 11, /**< Rotación del eje Z del giróscopo */

} hodor_ad_t;

typedef struct __attribute__((packed))
{
    uint8_t sof;

    uint8_t head;
    int16_t body;

    uint8_t checksum;

} hodor_msg_t;

void hodor_init();
void hodor_st_send();
void hodor_st_set_battery(uint16_t mv);
void hodor_st_set_encoders(int16_t left_steps, int16_t right_steps);
void hodor_st_set_motors(int16_t left_pwm, int16_t right_pwm);
void hodor_st_set_gyro(int16_t gz);
void hodor_msg_send(hodor_msg_t *msg);
void hodor_msg_recv(hodor_msg_t *msg);

#endif /* HODOR_H */
