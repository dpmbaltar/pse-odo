#include "hodor.h"
#include "serial.h"

hodor_st_t st;
sid32 sem;

void hodor_init(void)
{
    sem = semcreate(1);
}

void hodor_st_get(hodor_ad_t ad, int16_t *val)
{
    if (ad >= sizeof(hodor_st_t))
        return;

    wait(sem);
    *val = ((int16_t *)&st)[ad];
    signal(sem);
}

void hodor_st_set(hodor_ad_t ad, int16_t val)
{
    if (ad >= sizeof(hodor_st_t))
        return;

    wait(sem);
    ((int16_t *)&st)[ad] = val;
    signal(sem);
}

static uint8_t checksum8(const uint8_t *data, uint8_t len)
{
    uint8_t sum = 0;

    while (len--)
        sum ^= *data++;

    return sum;
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

int hodor_msg_recv(hodor_msg_t *msg)
{
    /*msg->sof = serial_get_char();
    if (msg->sof != SOF_RECV) {
        return -1;
    }*/
    do {
        msg->sof = serial_get_char();
    } while (msg->sof != SOF_RECV);

    size_t size = sizeof(hodor_msg_t);
    uint8_t *buffer = (uint8_t *)msg;
    uint8_t received = 1;
    while (received < size) {
        buffer[received] = serial_get_char();
        received++;
    }

    uint8_t chks = checksum8((uint8_t *)msg, sizeof(hodor_msg_t) - 1);
    if (chks != msg->checksum) {
        return -1;
    }

    return 0;
}
