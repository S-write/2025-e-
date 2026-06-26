/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define CPUCLK_FREQ                                                     32000000



/* Defines for PWMA */
#define PWMA_INST                                                          TIMG0
#define PWMA_INST_IRQHandler                                    TIMG0_IRQHandler
#define PWMA_INST_INT_IRQN                                      (TIMG0_INT_IRQn)
#define PWMA_INST_CLK_FREQ                                              32000000
/* GPIO defines for channel 0 */
#define GPIO_PWMA_C0_PORT                                                  GPIOA
#define GPIO_PWMA_C0_PIN                                          DL_GPIO_PIN_12
#define GPIO_PWMA_C0_IOMUX                                       (IOMUX_PINCM34)
#define GPIO_PWMA_C0_IOMUX_FUNC                      IOMUX_PINCM34_PF_TIMG0_CCP0
#define GPIO_PWMA_C0_IDX                                     DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWMA_C1_PORT                                                  GPIOA
#define GPIO_PWMA_C1_PIN                                          DL_GPIO_PIN_13
#define GPIO_PWMA_C1_IOMUX                                       (IOMUX_PINCM35)
#define GPIO_PWMA_C1_IOMUX_FUNC                      IOMUX_PINCM35_PF_TIMG0_CCP1
#define GPIO_PWMA_C1_IDX                                     DL_TIMER_CC_1_INDEX

/* Defines for PWM_0 */
#define PWM_0_INST                                                         TIMG7
#define PWM_0_INST_IRQHandler                                   TIMG7_IRQHandler
#define PWM_0_INST_INT_IRQN                                     (TIMG7_INT_IRQn)
#define PWM_0_INST_CLK_FREQ                                              1000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_0_C0_PORT                                                 GPIOA
#define GPIO_PWM_0_C0_PIN                                         DL_GPIO_PIN_28
#define GPIO_PWM_0_C0_IOMUX                                       (IOMUX_PINCM3)
#define GPIO_PWM_0_C0_IOMUX_FUNC                      IOMUX_PINCM3_PF_TIMG7_CCP0
#define GPIO_PWM_0_C0_IDX                                    DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_0_C1_PORT                                                 GPIOA
#define GPIO_PWM_0_C1_PIN                                         DL_GPIO_PIN_24
#define GPIO_PWM_0_C1_IOMUX                                      (IOMUX_PINCM54)
#define GPIO_PWM_0_C1_IOMUX_FUNC                     IOMUX_PINCM54_PF_TIMG7_CCP1
#define GPIO_PWM_0_C1_IDX                                    DL_TIMER_CC_1_INDEX



/* Defines for MOTOR_PID */
#define MOTOR_PID_INST                                                   (TIMA1)
#define MOTOR_PID_INST_IRQHandler                               TIMA1_IRQHandler
#define MOTOR_PID_INST_INT_IRQN                                 (TIMA1_INT_IRQn)
#define MOTOR_PID_INST_LOAD_VALUE                                       (15999U)
/* Defines for interrupt */
#define interrupt_INST                                                   (TIMA0)
#define interrupt_INST_IRQHandler                               TIMA0_IRQHandler
#define interrupt_INST_INT_IRQN                                 (TIMA0_INT_IRQn)
#define interrupt_INST_LOAD_VALUE                                        (3199U)




/* Defines for OLED */
#define OLED_INST                                                           I2C1
#define OLED_INST_IRQHandler                                     I2C1_IRQHandler
#define OLED_INST_INT_IRQN                                         I2C1_INT_IRQn
#define OLED_BUS_SPEED_HZ                                                 400000
#define GPIO_OLED_SDA_PORT                                                 GPIOB
#define GPIO_OLED_SDA_PIN                                          DL_GPIO_PIN_3
#define GPIO_OLED_IOMUX_SDA                                      (IOMUX_PINCM16)
#define GPIO_OLED_IOMUX_SDA_FUNC                       IOMUX_PINCM16_PF_I2C1_SDA
#define GPIO_OLED_SCL_PORT                                                 GPIOB
#define GPIO_OLED_SCL_PIN                                          DL_GPIO_PIN_2
#define GPIO_OLED_IOMUX_SCL                                      (IOMUX_PINCM15)
#define GPIO_OLED_IOMUX_SCL_FUNC                       IOMUX_PINCM15_PF_I2C1_SCL


/* Defines for UART_0 */
#define UART_0_INST                                                        UART0
#define UART_0_INST_FREQUENCY                                           32000000
#define UART_0_INST_IRQHandler                                  UART0_IRQHandler
#define UART_0_INST_INT_IRQN                                      UART0_INT_IRQn
#define GPIO_UART_0_RX_PORT                                                GPIOA
#define GPIO_UART_0_TX_PORT                                                GPIOA
#define GPIO_UART_0_RX_PIN                                        DL_GPIO_PIN_31
#define GPIO_UART_0_TX_PIN                                         DL_GPIO_PIN_0
#define GPIO_UART_0_IOMUX_RX                                      (IOMUX_PINCM6)
#define GPIO_UART_0_IOMUX_TX                                      (IOMUX_PINCM1)
#define GPIO_UART_0_IOMUX_RX_FUNC                       IOMUX_PINCM6_PF_UART0_RX
#define GPIO_UART_0_IOMUX_TX_FUNC                       IOMUX_PINCM1_PF_UART0_TX
#define UART_0_BAUD_RATE                                                (115200)
#define UART_0_IBRD_32_MHZ_115200_BAUD                                      (17)
#define UART_0_FBRD_32_MHZ_115200_BAUD                                      (23)





/* Port definition for Pin Group USER_SWITCHES_PA18 */
#define USER_SWITCHES_PA18_PORT                                          (GPIOA)

/* Defines for USER_SWITCH_PA18: GPIOA.18 with pinCMx 40 on package pin 11 */
#define USER_SWITCHES_PA18_USER_SWITCH_PA18_PIN                 (DL_GPIO_PIN_18)
#define USER_SWITCHES_PA18_USER_SWITCH_PA18_IOMUX                (IOMUX_PINCM40)
/* Port definition for Pin Group GPIO_SWITCHES_PB21 */
#define GPIO_SWITCHES_PB21_PORT                                          (GPIOB)

/* Defines for USER_SWITCH_1: GPIOB.21 with pinCMx 49 on package pin 20 */
#define GPIO_SWITCHES_PB21_USER_SWITCH_1_PIN                    (DL_GPIO_PIN_21)
#define GPIO_SWITCHES_PB21_USER_SWITCH_1_IOMUX                   (IOMUX_PINCM49)
/* Defines for AIN1: GPIOA.8 with pinCMx 19 on package pin 54 */
#define GPIO_MOTOR_AIN1_PORT                                             (GPIOA)
#define GPIO_MOTOR_AIN1_PIN                                      (DL_GPIO_PIN_8)
#define GPIO_MOTOR_AIN1_IOMUX                                    (IOMUX_PINCM19)
/* Defines for AIN2: GPIOA.9 with pinCMx 20 on package pin 55 */
#define GPIO_MOTOR_AIN2_PORT                                             (GPIOA)
#define GPIO_MOTOR_AIN2_PIN                                      (DL_GPIO_PIN_9)
#define GPIO_MOTOR_AIN2_IOMUX                                    (IOMUX_PINCM20)
/* Defines for DIN1: GPIOB.9 with pinCMx 26 on package pin 61 */
#define GPIO_MOTOR_DIN1_PORT                                             (GPIOB)
#define GPIO_MOTOR_DIN1_PIN                                      (DL_GPIO_PIN_9)
#define GPIO_MOTOR_DIN1_IOMUX                                    (IOMUX_PINCM26)
/* Defines for DIN2: GPIOA.27 with pinCMx 60 on package pin 31 */
#define GPIO_MOTOR_DIN2_PORT                                             (GPIOA)
#define GPIO_MOTOR_DIN2_PIN                                     (DL_GPIO_PIN_27)
#define GPIO_MOTOR_DIN2_IOMUX                                    (IOMUX_PINCM60)
/* Defines for AA: GPIOA.17 with pinCMx 39 on package pin 10 */
#define GPIO_MOTOR_AA_PORT                                               (GPIOA)
// pins affected by this interrupt request:["AA"]
#define GPIO_MOTOR_GPIOA_INT_IRQN                               (GPIOA_INT_IRQn)
#define GPIO_MOTOR_GPIOA_INT_IIDX               (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
#define GPIO_MOTOR_AA_IIDX                                  (DL_GPIO_IIDX_DIO17)
#define GPIO_MOTOR_AA_PIN                                       (DL_GPIO_PIN_17)
#define GPIO_MOTOR_AA_IOMUX                                      (IOMUX_PINCM39)
/* Defines for AB: GPIOB.0 with pinCMx 12 on package pin 47 */
#define GPIO_MOTOR_AB_PORT                                               (GPIOB)
#define GPIO_MOTOR_AB_PIN                                        (DL_GPIO_PIN_0)
#define GPIO_MOTOR_AB_IOMUX                                      (IOMUX_PINCM12)
/* Defines for DA: GPIOB.6 with pinCMx 23 on package pin 58 */
#define GPIO_MOTOR_DA_PORT                                               (GPIOB)
// pins affected by this interrupt request:["DA"]
#define GPIO_MOTOR_GPIOB_INT_IRQN                               (GPIOB_INT_IRQn)
#define GPIO_MOTOR_GPIOB_INT_IIDX               (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define GPIO_MOTOR_DA_IIDX                                   (DL_GPIO_IIDX_DIO6)
#define GPIO_MOTOR_DA_PIN                                        (DL_GPIO_PIN_6)
#define GPIO_MOTOR_DA_IOMUX                                      (IOMUX_PINCM23)
/* Defines for DB: GPIOB.7 with pinCMx 24 on package pin 59 */
#define GPIO_MOTOR_DB_PORT                                               (GPIOB)
#define GPIO_MOTOR_DB_PIN                                        (DL_GPIO_PIN_7)
#define GPIO_MOTOR_DB_IOMUX                                      (IOMUX_PINCM24)
/* Port definition for Pin Group TRACT */
#define TRACT_PORT                                                       (GPIOB)

/* Defines for AD0: GPIOB.13 with pinCMx 30 on package pin 1 */
#define TRACT_AD0_PIN                                           (DL_GPIO_PIN_13)
#define TRACT_AD0_IOMUX                                          (IOMUX_PINCM30)
/* Defines for AD1: GPIOB.17 with pinCMx 43 on package pin 14 */
#define TRACT_AD1_PIN                                           (DL_GPIO_PIN_17)
#define TRACT_AD1_IOMUX                                          (IOMUX_PINCM43)
/* Defines for AD2: GPIOB.15 with pinCMx 32 on package pin 3 */
#define TRACT_AD2_PIN                                           (DL_GPIO_PIN_15)
#define TRACT_AD2_IOMUX                                          (IOMUX_PINCM32)
/* Defines for AD_OUT: GPIOB.16 with pinCMx 33 on package pin 4 */
#define TRACT_AD_OUT_PIN                                        (DL_GPIO_PIN_16)
#define TRACT_AD_OUT_IOMUX                                       (IOMUX_PINCM33)




/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_PWMA_init(void);
void SYSCFG_DL_PWM_0_init(void);
void SYSCFG_DL_MOTOR_PID_init(void);
void SYSCFG_DL_interrupt_init(void);
void SYSCFG_DL_OLED_init(void);
void SYSCFG_DL_UART_0_init(void);

void SYSCFG_DL_SYSTICK_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
