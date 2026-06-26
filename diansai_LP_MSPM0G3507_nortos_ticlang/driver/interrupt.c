#include "interrupt.h"

uint32_t counter_1_A = 0;
uint32_t counter_1_B = 0;

void GROUP1_IRQHandler(void)
{
    switch (DL_GPIO_getPendingInterrupt(GPIOA)) {
        case GPIO_MOTOR_AA_IIDX:
            counter_1_A++;
            break;
        default:
            break;
    }
    switch (DL_GPIO_getPendingInterrupt(GPIOB)) {
        case GPIO_MOTOR_DA_IIDX:
            counter_1_B++;
            break;
        default:
            break;
    }
}

void SysTick_Handler(void)
{
    g_ms_tick++;
}

void interrupt_INST_IRQHandler(void)
{
    switch (DL_Timer_getPendingInterrupt(interrupt_INST)) {
        case DL_TIMER_IIDX_LOAD:
            gray_data_ready = 0;
            break;
        default:
            break;
    }
}
