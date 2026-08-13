/**
 * @file timer.h
 * @author Diego Pablo Matías Baltar <diego.baltar@est.fi.uncoma.edu.ar>
 * @brief Timer utility functions
 * @date 2024-06-13
 * @version 0.1
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

void timer1_init(void);
void timer1_set_pwm_A(uint8_t width);
void timer1_set_pwm_B(uint8_t width);

#endif /* _TIMER_H_ */
