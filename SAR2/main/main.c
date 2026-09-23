// Joystick-controlled servo (ESP32 + ESP-IDF)
//
// Wiring (see diagram.json):
//   Joystick HORZ -> GPIO34  (ADC1_CH6)   X axis  -> servo angle
//   Joystick VERT -> GPIO35  (ADC1_CH7)   Y axis  -> read & printed (spare)
//   Joystick SEL  -> GPIO19  (active low) press   -> recenter to 90 deg
//   Servo PWM     -> GPIO18  (LEDC, 50 Hz)

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_idf_version.h"

// ---------------- configuration ----------------
#define SERVO_GPIO          18
#define JOY_SEL_GPIO        19
#define JOY_X_CHANNEL       ADC_CHANNEL_6      // GPIO34
#define JOY_Y_CHANNEL       ADC_CHANNEL_7      // GPIO35

#define SERVO_MIN_PULSE_US  500                // 0 deg
#define SERVO_MAX_PULSE_US  2500               // 180 deg
#define SERVO_FREQ_HZ       50                 // 20 ms period
#define SERVO_RES           LEDC_TIMER_14_BIT  // 16384 steps per period
#define SERVO_MAX_DUTY      ((1 << 14) - 1)

#define ADC_MAX             4095
#define ADC_CENTER          2048
#define ADC_DEADZONE        150                // ignore jitter around center
#define SMOOTHING           0.20f              // 0..1, lower = smoother
#define LOOP_PERIOD_MS      20

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 0)
#define JOY_ADC_ATTEN       ADC_ATTEN_DB_12
#else
#define JOY_ADC_ATTEN       ADC_ATTEN_DB_11
#endif

static adc_oneshot_unit_handle_t s_adc1;

// ---------------- servo ----------------
static void servo_init(void)
{
    ledc_timer_config_t timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = LEDC_TIMER_0,
        .duty_resolution = SERVO_RES,
        .freq_hz         = SERVO_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t ch = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = SERVO_GPIO,
        .duty       = 0,
        .hpoint     = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch));
}

// angle: 0.0 .. 180.0 degrees
static void servo_write_angle(float angle)
{
    if (angle < 0.0f)   angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;

    float pulse_us = SERVO_MIN_PULSE_US +
                     (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US) * (angle / 180.0f);

    // duty = pulse / period, period = 1e6 / freq microseconds
    uint32_t duty = (uint32_t)((pulse_us * SERVO_FREQ_HZ * (SERVO_MAX_DUTY + 1)) / 1000000.0f);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

// ---------------- joystick ----------------
static void joystick_init(void)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = { .unit_id = ADC_UNIT_1 };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &s_adc1));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten    = JOY_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc1, JOY_X_CHANNEL, &chan_cfg));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc1, JOY_Y_CHANNEL, &chan_cfg));

    gpio_config_t btn = {
        .pin_bit_mask = 1ULL << JOY_SEL_GPIO,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&btn));
}

// average a few samples to knock down ADC noise
static int joystick_read(adc_channel_t ch)
{
    int sum = 0, raw = 0;
    for (int i = 0; i < 8; i++) {
        adc_oneshot_read(s_adc1, ch, &raw);
        sum += raw;
    }
    return sum / 8;
}

// raw ADC -> 0..180 deg, with a deadzone that holds dead centre at 90
static float axis_to_angle(int raw)
{
    int offset = raw - ADC_CENTER;
    if (abs(offset) < ADC_DEADZONE) return 90.0f;

    // re-scale so the angle is continuous at the edge of the deadzone
    if (offset > 0) offset -= ADC_DEADZONE;
    else            offset += ADC_DEADZONE;

    float span = (float)(ADC_CENTER - ADC_DEADZONE);
    float norm = (float)offset / span;          // -1 .. +1
    if (norm < -1.0f) norm = -1.0f;
    if (norm >  1.0f) norm =  1.0f;

    return 90.0f + norm * 90.0f;
}

// ---------------- main ----------------
void app_main(void)
{
    servo_init();
    joystick_init();

    printf("Joystick -> servo ready. Move the stick; press it to recenter.\n");

    float angle = 90.0f;
    servo_write_angle(angle);
    vTaskDelay(pdMS_TO_TICKS(300));

    int tick = 0;

    while (true) {
        int x_raw = joystick_read(JOY_X_CHANNEL);
        int y_raw = joystick_read(JOY_Y_CHANNEL);
        bool pressed = (gpio_get_level(JOY_SEL_GPIO) == 0);

        float target = pressed ? 90.0f : axis_to_angle(x_raw);

        // exponential smoothing so the horn sweeps instead of snapping
        angle += (target - angle) * SMOOTHING;
        servo_write_angle(angle);

        if (++tick >= 10) {   // ~5 prints per second
            tick = 0;
            printf("X=%4d  Y=%4d  SEL=%d  ->  angle=%5.1f deg\n",
                   x_raw, y_raw, pressed ? 1 : 0, angle);
        }

        vTaskDelay(pdMS_TO_TICKS(LOOP_PERIOD_MS));
    }
}
