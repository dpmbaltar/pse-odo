#include "pid.h"

void pid_init(pid_t  *pid,
              int16_t kp,
              int16_t ki,
              int16_t kd,
              int16_t integral_limit,
              int16_t output_limit)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->integral = 0;
    pid->previous_error = 0;

    pid->integral_limit = integral_limit;
    pid->output_limit = output_limit;
}


int16_t pid_update(pid_t *pid,
                   int16_t setpoint,
                   int16_t feedback)
{
    int16_t error;
    int16_t derivative;
    int32_t output;

    error = setpoint - feedback;

    /*
     * Integral con anti-windup.
     */
    pid->integral += error;
    if (pid->integral > pid->integral_limit) {
        pid->integral = pid->integral_limit;
    } else if (pid->integral < -pid->integral_limit) {
        pid->integral = -pid->integral_limit;
    }

    derivative = error - pid->previous_error;
    pid->previous_error = error;

    /*
     * El resultado puede superar los 16 bits, por eso output es int32_t.
     */
    output = (int32_t)pid->kp * error +
             (int32_t)pid->ki * pid->integral +
             (int32_t)pid->kd * derivative;

    /*
     * Escala de las ganancias fraccionarias.
     * Por ejemplo, si Kp = 1.5: 1.5 * 256 = 384, es decir, Kp = 384
     * De esta forma se evita el uso de float.
     */
    output >>= 8;

    if (output > pid->output_limit) {
        output = pid->output_limit;
    } else if (output < -pid->output_limit) {
        output = -pid->output_limit;
    }

    return (int16_t)output;
}
