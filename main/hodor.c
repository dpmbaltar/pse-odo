#include "hodor.h"

hodor_state_t hodor_state = HODOR_STATE_INIT;
sid32 hodor_sem;

static uint8_t checksum8(const uint8_t *data, uint8_t len)
{
    uint8_t sum = 0;

    while (len--)
        sum ^= *data++;

    return sum;
}

void hodor_init(void)
{
    hodor_sem = semcreate(1);
}

void hodor_state_send()
{
    hodor_state_t data;
    uint8_t *buffer = (uint8_t *)&data;
    size_t size = sizeof(hodor_state_t);

    wait(hodor_sem);
    data.sof = hodor_state.sof;
    data.l_encoder_steps = hodor_state.l_encoder_steps;
    data.r_encoder_steps = hodor_state.r_encoder_steps;
    data.battery_mv = hodor_state.battery_mv;
    //data.gyro_x = hodor_state.gyro_x;
    //data.gyro_y = hodor_state.gyro_y;
    data.gyro_z = hodor_state.gyro_z; //enviar cada 100hz
    signal(hodor_sem);

    data.checksum = checksum8(buffer, size - 1);

    for (int i = 0; i < size; i++) {
        serial_put_char(buffer[i]);
    }
}

void hodor_state_set_encoders(int16_t left_steps, int16_t right_steps)
{
    wait(hodor_sem);

    hodor_state.l_encoder_steps = left_steps;
    hodor_state.r_encoder_steps = right_steps;

    signal(hodor_sem);
}

void hodor_state_set_gyro(int16_t gx, int16_t gy, int16_t gz)
{
    wait(hodor_sem);

    //hodor_state.gyro_x = gx;
    //hodor_state.gyro_y = gy;
    hodor_state.gyro_z = gz;

    signal(hodor_sem);
}
