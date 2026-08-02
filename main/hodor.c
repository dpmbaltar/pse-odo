#include "hodor.h"
#include "serial.h"

#define HODOR_ST_INIT {0, 0, 0, 0, 0, 0, 0, 0}

hodor_st_t st = HODOR_ST_INIT;
sid32 sem;

static uint8_t checksum8(const uint8_t *data, uint8_t len)
{
    uint8_t sum = 0;

    while (len--)
        sum ^= *data++;

    return sum;
}

void hodor_init(void)
{
    sem = semcreate(1);
}

void hodor_st_send()
{
    hodor_st_t data = {.sof = SOF_SEND};
    uint8_t *buffer = (uint8_t *)&data;
    size_t size = sizeof(hodor_st_t);

    wait(sem);
    data.l_encoder_steps = st.l_encoder_steps;
    data.r_encoder_steps = st.r_encoder_steps;
    data.l_motor_pwm = st.l_motor_pwm;
    data.r_motor_pwm = st.r_motor_pwm;
    data.battery_mv = st.battery_mv;
    data.gyro_z = st.gyro_z;
    signal(sem);

    data.checksum = checksum8(buffer, size - 1);

    for (int i = 0; i < size; i++) {
        serial_put_char(buffer[i]);
    }
}

void hodor_st_set_encoders(int16_t left_steps, int16_t right_steps)
{
    wait(sem);
    st.l_encoder_steps = left_steps;
    st.r_encoder_steps = right_steps;
    signal(sem);
}

void hodor_st_set_gyro(int16_t gz)
{
    wait(sem);
    st.gyro_z = gz;
    signal(sem);
}

void hodor_msg_send(hodor_msg_t *msg)
{
    uint8_t *buffer = (uint8_t *)msg;
    size_t size = sizeof(hodor_msg_t);

    msg->sof = SOF_SEND;
    msg->checksum = checksum8((uint8_t *)msg, sizeof(hodor_msg_t) - 1);

    for (int i = 0; i < size; i++) {
        serial_put_char(buffer[i]);
    }
}

void hodor_msg_recv(hodor_msg_t *msg)
{
    msg->sof = serial_get_char();
    if (msg->sof != SOF_RECV) {
        msg->head = 0;
        msg->body = 0;
        msg->checksum = 0;
        return;
    }

    size_t size = sizeof(hodor_msg_t);
    uint8_t *buffer = (uint8_t *)msg;
    uint8_t received = 1;
    while (received < size) {
        buffer[received] = serial_get_char();
        received++;
    }

    uint8_t chks = checksum8((uint8_t *)msg, sizeof(hodor_msg_t) - 1);
    if (chks != msg->checksum) {
        msg->head = 0;
        msg->body = 0;
        msg->checksum = 0;
    }
}
