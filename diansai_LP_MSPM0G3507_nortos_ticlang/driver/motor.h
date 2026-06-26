#ifndef MOTOR_H
#define MOTOR_H
#include "ti_msp_dl_config.h"

#define PI 3.14
#define MOTOR_BIANMAQI 260
#define MOTOR_WHEEL_D 48

void motor_init(uint8_t motor_id);
void motor_set_duty(uint8_t motor_id,uint32_t duty);
void motor_set_direction(uint8_t motor_id,uint8_t direction);

extern uint8_t corner_mode;
void motor_reset_pid(void);

#endif
