#ifndef GRAY_SENSOR_H
#define GRAY_SENSOR_H

#include "ti_msp_dl_config.h"

#define GRAY_PORT             TRACT_PORT
#define GRAY_AD0_PIN          TRACT_AD0_PIN
#define GRAY_AD1_PIN          TRACT_AD1_PIN
#define GRAY_AD2_PIN          TRACT_AD2_PIN
#define GRAY_OUT_PORT         TRACT_PORT
#define GRAY_OUT_PIN          TRACT_AD_OUT_PIN

#define GRAY_CHANNEL_NUM      8
#define GRAY_US_DELAY         50

#define GRAY_WEIGHT_CH0   (-450.0f)
#define GRAY_WEIGHT_CH1   (-200.0f)
#define GRAY_WEIGHT_CH2   (-15.0f)
#define GRAY_WEIGHT_CH3   (-2.0f)
#define GRAY_WEIGHT_CH4   (5.0f)
#define GRAY_WEIGHT_CH5   (15.0f)
#define GRAY_WEIGHT_CH6   (200.0f)
#define GRAY_WEIGHT_CH7   (450.0f)

extern volatile uint8_t  gray_data[GRAY_CHANNEL_NUM];
extern volatile uint8_t  gray_data_ready;
extern volatile uint32_t g_ms_tick;

void gray_sensor_init(void);
void Gray_Task_Loop(void);
int16_t Gray_Calc_Track_Error(void);

#endif
