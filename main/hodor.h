#ifndef HODOR_H
#define HODOR_H

#include <xinu.h>

#define HODOR_SOF_RECV 0x55
#define HODOR_SOF_SEND 0xAA

#define HODOR_STATE_INIT { HODOR_SOF_SEND, 0UL, 0, 0, 0, 0, 0U, /*0, 0,*/ 0, 0U }

#pragma pack(push, 1)

typedef struct
{
    uint8_t sof; // Start of frame

    //uint32_t timestamp_ms;

    int16_t l_encoder_steps;
    int16_t r_encoder_steps;

    int16_t l_motor_pwm;
    int16_t r_motor_pwm;

    //int16_t l_motor_target_pwm;
    //int16_t r_motor_target_pwm;

    uint16_t battery_mv;

    //int16_t gyro_x;
    //int16_t gyro_y;
    int16_t gyro_z; //Solo z?

    uint8_t checksum;

} hodor_state_t;

typedef struct
{
    uint8_t sof;

    int16_t l_motor_target_pwm;
    int16_t r_motor_target_pwm;

    uint8_t checksum;

} hodor_cmd_t;

#pragma pack(pop)

void hodor_init();
void hodor_state_send();
void hodor_state_set_battery(uint16_t mv);
void hodor_state_set_encoders(int16_t left_steps, int16_t right_steps);
void hodor_state_set_motors(int16_t left_pwm, int16_t right_pwm);
void hodor_state_set_gyro(int16_t gx, int16_t gy, int16_t gz);

#endif /* HODOR_H */
