#pragma once
#include "driver/gpio.h"

/* 引脚定義 */
constexpr gpio_num_t PIN_BATTERY = GPIO_NUM_2;
constexpr gpio_num_t PIN_BAT_LED1 = GPIO_NUM_11;
constexpr gpio_num_t PIN_BAT_LED2 = GPIO_NUM_10;

/* 電池電阻配置 */
constexpr float R_UP = 100000.0;   // 上拉電阻
constexpr float R_DOWN = 360000.0; // 下拉電阻