#include "motor.h"
void motor_init(uint8_t motor_id)
{
    if(motor_id==1)
    {
        DL_Timer_startCounter(PWMA_INST);
        DL_GPIO_setPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
        DL_GPIO_setPins(GPIO_MOTOR_AIN2_PORT,GPIO_MOTOR_AIN2_PIN);
        DL_Timer_setCaptureCompareValue(PWMA_INST, 0, GPIO_PWMA_C0_IDX);
    }
    else if (motor_id==2) 
    {
        DL_Timer_startCounter(PWMA_INST);
        DL_GPIO_setPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
        DL_GPIO_setPins(GPIO_MOTOR_DIN2_PORT,GPIO_MOTOR_DIN2_PIN);
        DL_Timer_setCaptureCompareValue(PWMA_INST, 0, GPIO_PWMA_C1_IDX);
    }
    DL_Timer_startCounter(MOTOR_PID_INST);
    NVIC_EnableIRQ(MOTOR_PID_INST_INT_IRQN);
}

void motor_set_duty(uint8_t motor_id,uint32_t duty)
{
    if(duty>3200) duty=3200;
    if(motor_id==1)
        DL_Timer_setCaptureCompareValue(PWMA_INST,duty,GPIO_PWMA_C0_IDX);
    else if(motor_id==2)
        DL_Timer_setCaptureCompareValue(PWMA_INST,duty,GPIO_PWMA_C1_IDX);
}

void motor_set_direction(uint8_t motor_id,uint8_t direction)
{
    if(motor_id==1)
    {
        if(direction==0)
        {
            DL_GPIO_setPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
            DL_GPIO_setPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
        }
        else if(direction==1)
        {
            DL_GPIO_setPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
            DL_GPIO_clearPins(GPIO_MOTOR_AIN1_PORT,GPIO_MOTOR_AIN1_PIN);
        }
        else if(direction==2)
        {
            DL_GPIO_setPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
            DL_GPIO_clearPins(GPIO_MOTOR_AIN2_PORT,GPIO_MOTOR_AIN2_PIN);
        }
    }
    else if(motor_id==2)
    {
        if(direction==0)
        {
            DL_GPIO_setPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
            DL_GPIO_setPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
        }
        else if(direction==1)
        {
            DL_GPIO_setPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
            DL_GPIO_clearPins(GPIO_MOTOR_DIN1_PORT,GPIO_MOTOR_DIN1_PIN);
        }
        else if(direction==2)
        {
            DL_GPIO_setPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
            DL_GPIO_clearPins(GPIO_MOTOR_DIN2_PORT,GPIO_MOTOR_DIN2_PIN);
        }
    }
}

extern uint32_t counter_1_A;
extern uint32_t counter_1_B;
float speed_1=0.0;
float speed_2=0.0;
void calculate_speed(uint8_t motor_id)
{
    if(motor_id==1)
    {
        speed_1=(float)counter_1_A/MOTOR_BIANMAQI*PI*MOTOR_WHEEL_D*20;
        counter_1_A=0;
    }
    if(motor_id==2)
    {
        speed_2=(float)counter_1_B/MOTOR_BIANMAQI*PI*MOTOR_WHEEL_D*20;
        counter_1_B=0;
    }
}

float Kp=1.00;
float Ki=0.80;
int32_t PWM_1_duty = 0;
int32_t PWM_2_duty = 0;
float target_speed_1=0;
float target_speed_2=0;
float last_error_1=0;
float current_error_1=0;
float last_error_2=0;
float current_error_2=0;
uint8_t corner_mode = 0;

void DC_MOTOR_PID(uint8_t motor_id)
{
    float error;
    if(motor_id==1){
        error=target_speed_1-speed_1;
        current_error_1=error;
        PWM_1_duty+=(uint16_t)(Kp*(current_error_1-last_error_1)+Ki*(current_error_1));
        last_error_1=current_error_1;
        if (PWM_1_duty < 0) PWM_1_duty = 0; motor_set_duty(motor_id, (uint32_t)PWM_1_duty);
    }
    else if (motor_id==2) 
    {
        error=target_speed_2-speed_2;
        current_error_2=error;
        PWM_2_duty+=(uint16_t)(Kp*(current_error_2-last_error_2)+Ki*(current_error_2));
        last_error_2=current_error_2;
        if (PWM_2_duty < 0) PWM_2_duty = 0; motor_set_duty(motor_id, (uint32_t)PWM_2_duty);
    }
}

void MOTOR_PID_INST_IRQHandler()
{
    switch (DL_Timer_getPendingInterrupt(MOTOR_PID_INST)) 
    {
        case DL_TIMER_IIDX_LOAD:
            if (!corner_mode) {
                calculate_speed(1);
                calculate_speed(2);
                DC_MOTOR_PID(1);
                DC_MOTOR_PID(2);
            }
            break;
        default:
            break;
    }
}

void motor_reset_pid(void)
{
    counter_1_A = 0;
    counter_1_B = 0;
    /* PWM_1_duty 不清零，让 PID 平滑接手 */
    /* PWM_2_duty 不清零 */
    last_error_1 = 0;
    current_error_1 = 0;
    last_error_2 = 0;
    current_error_2 = 0;
}

