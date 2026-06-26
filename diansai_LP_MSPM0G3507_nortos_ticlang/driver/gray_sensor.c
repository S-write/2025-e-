#include "gray_sensor.h"

/* ============ ???? ============ */
volatile uint8_t  gray_data[GRAY_CHANNEL_NUM] = {0};
volatile uint8_t  gray_data_ready = 0;
volatile uint32_t g_ms_tick = 0;

/* ============ ?????? ============ */
static const float gray_weight[GRAY_CHANNEL_NUM] = {
    GRAY_WEIGHT_CH0, GRAY_WEIGHT_CH1, GRAY_WEIGHT_CH2, GRAY_WEIGHT_CH3,
    GRAY_WEIGHT_CH4, GRAY_WEIGHT_CH5, GRAY_WEIGHT_CH6, GRAY_WEIGHT_CH7
};

static int16_t last_valid_error = 0;

/* ??????? */
static uint32_t delay_start_tick = 0;
static uint8_t  delay_pending = 0;

/* ??? */
typedef enum {
    GRAY_STATE_IDLE = 0,
    GRAY_STATE_CH0, GRAY_STATE_CH1, GRAY_STATE_CH2, GRAY_STATE_CH3,
    GRAY_STATE_CH4, GRAY_STATE_CH5, GRAY_STATE_CH6, GRAY_STATE_CH7,
    GRAY_STATE_DONE
} Gray_Read_State_t;

static Gray_Read_State_t read_state = GRAY_STATE_IDLE;

/* ============ ??????????? SysTick?============ */
static uint32_t Get_System_Tick_us(void)
{
    uint32_t ms  = g_ms_tick;
    uint32_t val = SysTick->VAL;
    uint32_t load = SysTick->LOAD;
    return ms * 1000 + (load - val) / 32;
}

static void Delay_Start_NonBlocking(uint32_t us)
{
    delay_start_tick = Get_System_Tick_us();
    delay_pending = 1;
}

static uint8_t Delay_Check_NonBlocking(uint32_t us)
{
    if (delay_pending && (Get_System_Tick_us() - delay_start_tick >= us)) {
        delay_pending = 0;
        return 1;
    }
    return 0;
}

void gray_sensor_init(void)
{
    DL_GPIO_initDigitalOutput(TRACT_AD0_IOMUX);
    DL_GPIO_initDigitalOutput(TRACT_AD1_IOMUX);
    DL_GPIO_initDigitalOutput(TRACT_AD2_IOMUX);
    DL_GPIO_initDigitalInput(TRACT_AD_OUT_IOMUX);
    DL_GPIO_clearPins(GRAY_PORT, GRAY_AD0_PIN | GRAY_AD1_PIN | GRAY_AD2_PIN);
}

static void Gray_Select_Channel(uint8_t ch)
{
    if (ch >= GRAY_CHANNEL_NUM) return;
    (ch & 0x01)       ? DL_GPIO_setPins(GRAY_PORT, GRAY_AD0_PIN)
                       : DL_GPIO_clearPins(GRAY_PORT, GRAY_AD0_PIN);
    ((ch >> 1) & 0x01) ? DL_GPIO_setPins(GRAY_PORT, GRAY_AD1_PIN)
                       : DL_GPIO_clearPins(GRAY_PORT, GRAY_AD1_PIN);
    ((ch >> 2) & 0x01) ? DL_GPIO_setPins(GRAY_PORT, GRAY_AD2_PIN)
                       : DL_GPIO_clearPins(GRAY_PORT, GRAY_AD2_PIN);
}

void Gray_Task_Loop(void)
{
    switch (read_state) {
    case GRAY_STATE_IDLE:
        if (gray_data_ready == 0) {
            gray_data_ready = 1;
            Gray_Select_Channel(0);
            Delay_Start_NonBlocking(GRAY_US_DELAY);
            read_state = GRAY_STATE_CH0;
        }
        break;
    case GRAY_STATE_CH0:
        if (Delay_Check_NonBlocking(GRAY_US_DELAY)) {
            gray_data[0] = DL_GPIO_readPins(GRAY_OUT_PORT, GRAY_OUT_PIN) ? 1 : 0;
            Gray_Select_Channel(1); Delay_Start_NonBlocking(GRAY_US_DELAY);
            read_state = GRAY_STATE_CH1;
        } break;
    case GRAY_STATE_CH1:
        if (Delay_Check_NonBlocking(GRAY_US_DELAY)) {
            gray_data[1] = DL_GPIO_readPins(GRAY_OUT_PORT, GRAY_OUT_PIN) ? 1 : 0;
            Gray_Select_Channel(2); Delay_Start_NonBlocking(GRAY_US_DELAY);
            read_state = GRAY_STATE_CH2;
        } break;
    case GRAY_STATE_CH2:
        if (Delay_Check_NonBlocking(GRAY_US_DELAY)) {
            gray_data[2] = DL_GPIO_readPins(GRAY_OUT_PORT, GRAY_OUT_PIN) ? 1 : 0;
            Gray_Select_Channel(3); Delay_Start_NonBlocking(GRAY_US_DELAY);
            read_state = GRAY_STATE_CH3;
        } break;
    case GRAY_STATE_CH3:
        if (Delay_Check_NonBlocking(GRAY_US_DELAY)) {
            gray_data[3] = DL_GPIO_readPins(GRAY_OUT_PORT, GRAY_OUT_PIN) ? 1 : 0;
            Gray_Select_Channel(4); Delay_Start_NonBlocking(GRAY_US_DELAY);
            read_state = GRAY_STATE_CH4;
        } break;
    case GRAY_STATE_CH4:
        if (Delay_Check_NonBlocking(GRAY_US_DELAY)) {
            gray_data[4] = DL_GPIO_readPins(GRAY_OUT_PORT, GRAY_OUT_PIN) ? 1 : 0;
            Gray_Select_Channel(5); Delay_Start_NonBlocking(GRAY_US_DELAY);
            read_state = GRAY_STATE_CH5;
        } break;
    case GRAY_STATE_CH5:
        if (Delay_Check_NonBlocking(GRAY_US_DELAY)) {
            gray_data[5] = DL_GPIO_readPins(GRAY_OUT_PORT, GRAY_OUT_PIN) ? 1 : 0;
            Gray_Select_Channel(6); Delay_Start_NonBlocking(GRAY_US_DELAY);
            read_state = GRAY_STATE_CH6;
        } break;
    case GRAY_STATE_CH6:
        if (Delay_Check_NonBlocking(GRAY_US_DELAY)) {
            gray_data[6] = DL_GPIO_readPins(GRAY_OUT_PORT, GRAY_OUT_PIN) ? 1 : 0;
            Gray_Select_Channel(7); Delay_Start_NonBlocking(GRAY_US_DELAY);
            read_state = GRAY_STATE_CH7;
        } break;
    case GRAY_STATE_CH7:
        if (Delay_Check_NonBlocking(GRAY_US_DELAY)) {
            gray_data[7] = DL_GPIO_readPins(GRAY_OUT_PORT, GRAY_OUT_PIN) ? 1 : 0;
            gray_data_ready = 2;
            read_state = GRAY_STATE_DONE;
        } break;
    case GRAY_STATE_DONE:
        read_state = GRAY_STATE_IDLE;
        break;
    default:
        read_state = GRAY_STATE_IDLE;
        break;
    }
}

int16_t Gray_Calc_Track_Error(void)
{
    float sum_error = 0.0f;
    int32_t sum_weight = 0;

    for (uint8_t i = 0; i < GRAY_CHANNEL_NUM; i++) {
        if (gray_data[i]) {
            sum_error  += gray_weight[i];
            sum_weight += 1;
        }
    }

    if (sum_weight == 0) {
        return last_valid_error;
    }

    int16_t current_error = (int16_t)(sum_error * 10.0f / (float)sum_weight);
    last_valid_error = current_error;
    return current_error;
}
