#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

#define MPU6050_ADDR 0x68

typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;

    int16_t temp;

    int16_t gx;
    int16_t gy;
    int16_t gz;

} mpu6050_data_t;

void mpu6050_init(void);
void mpu6050_read(mpu6050_data_t *data);

#endif

