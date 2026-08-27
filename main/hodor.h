#ifndef HODOR_H
#define HODOR_H

#include <xinu.h>

#define SOF_RECV 0x55 /**< Arduino <= Linux */
#define SOF_SEND 0xAA /**< Arduino => Linux */

#define AD_MASK  0xFC /**< Máscara del offset del byte del estado */
#define OP_MASK  0x03 /**< Máscara de la operación sobre el estado */

#define OP_ACK   0
#define OP_READ  1    /**< Operación de lectura */
#define OP_WRITE 2    /**< Operación de escritura */

#define MSG_INIT {0, 0, {0}, 0}

#define MSG_HEAD(op, ad) \
    (((((uint8_t)(ad)) << 2) & AD_MASK) | (((uint8_t)(op)) & OP_MASK))
#define MSG_ADDR(h) \
    ((((uint8_t)(h)) & AD_MASK) >> 2)
#define MSG_OP(h) \
    (((uint8_t)(h)) & OP_MASK)

/** Estado del HODOR */
typedef struct __attribute__((packed))
{
    int16_t lenc_steps;  /**< Pasos del encoder izquierdo */
    int16_t renc_steps;  /**< Pasos del encoder derecho */

    int16_t lmotor_pwm;  /**< PWM del motor izquierdo [-255:255] */
    int16_t rmotor_pwm;  /**< PWM del motor derecho [-255:255] */

    int16_t lmotor_tpwm; /**< PWM objetivo del motor izquierdo */
    int16_t rmotor_tpwm; /**< PWM objetivo del motor derecho */

    uint16_t battery_mv; /**< Estado de batería (mV) */

    int16_t gyro_z;      /**< Rotación del eje Z del giróscopo */

} hodor_st_t;

/** Offsets para acceder al estado del HODOR */
typedef enum
{
    LENC_STEPS,  /**< Pasos del encoder izquierdo */
    RENC_STEPS,  /**< Pasos del encoder derecho */
    LMOTOR_PWM,  /**< PWM del motor izquierdo */
    RMOTOR_PWM,  /**< PWM del motor derecho */
    LMOTOR_TPWM, /**< PWM objetivo del motor izquierdo */
    RMOTOR_TPWM, /**< PWM objetivo del motor derecho */
    BATTERY_MV,  /**< Estado de batería */
    GYRO_Z,      /**< Rotación del eje Z del giróscopo */

} hodor_ad_t;

/** Mensaje para la comunicación con el HODOR */
typedef struct __attribute__((packed))
{
    uint8_t sof;  /**< Start Of Frame */
    uint8_t head; /**< Encabezado del mensaje */

    union {
        int16_t body;

        int16_t lenc_steps;
        int16_t renc_steps;
        int16_t lmotor_pwm;
        int16_t rmotor_pwm;
        int16_t lmotor_tpwm;
        int16_t rmotor_tpwm;
        uint16_t battery_mv;
        int16_t gyro_z;
    };

    uint8_t checksum;

} hodor_msg_t;

void hodor_init(void);
void hodor_st_get(hodor_ad_t ad, int16_t *val);
void hodor_st_set(hodor_ad_t ad, int16_t val);
void hodor_msg_send(hodor_msg_t *msg);
int  hodor_msg_recv(hodor_msg_t *msg);

#endif /* HODOR_H */
