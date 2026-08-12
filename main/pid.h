#ifndef PID_H
#define PID_H

#include <stdint.h>

typedef struct
{
    int16_t kp;             /**< K Proporcional */
    int16_t ki;             /**< K Integral */
    int16_t kd;             /**< K Diferencial */

    int32_t integral;       /**< Integración de errores */
    int16_t previous_error; /**< Error anterior */

    int16_t integral_limit; /**< Límite integral inferior/superior */
    int16_t output_limit;   /**< Límite de la salida */

} pid_t;

void pid_init(pid_t  *pid,
              int16_t kp,
              int16_t ki,
              int16_t kd,
              int16_t integral_limit,
              int16_t output_limit);

int16_t pid_update(pid_t  *pid,
                   int16_t setpoint,
                   int16_t feedback);

#endif
