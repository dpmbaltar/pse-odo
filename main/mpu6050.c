#include "mpu6050.h"
#include "twi.h"

#define MPU6050_PWR_MGMT_1   0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

static void mpu6050_write_reg(uint8_t reg, uint8_t value)
{
    twi_start((MPU6050_ADDR << 1) | 0);
    twi_write(reg);
    twi_write(value);
    twi_stop();
}

static void mpu6050_read_regs(uint8_t reg, uint8_t *buffer, uint8_t len)
{
    twi_start((MPU6050_ADDR << 1) | 0);
    twi_write(reg);
    twi_start((MPU6050_ADDR << 1) | 1);

    for (uint8_t i = 0; i < (len - 1); i++)
        buffer[i] = twi_read_ack();

    buffer[len - 1] = twi_read_nack();

    twi_stop();
}

void mpu6050_init(void)
{
    /* Sale del modo sleep */
    mpu6050_write_reg(MPU6050_PWR_MGMT_1, 0x00);
}

void mpu6050_read(mpu6050_data_t *data)
{
    uint8_t raw[14];

    mpu6050_read_regs(MPU6050_ACCEL_XOUT_H, raw, 14);

    data->ax = (raw[0] << 8) | raw[1];
    data->ay = (raw[2] << 8) | raw[3];
    data->az = (raw[4] << 8) | raw[5];

    data->temp = (raw[6] << 8) | raw[7];

    data->gx = (raw[8] << 8) | raw[9];
    data->gy = (raw[10] << 8) | raw[11];
    data->gz = (raw[12] << 8) | raw[13];
}

