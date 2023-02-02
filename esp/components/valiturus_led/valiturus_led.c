/*  WS2812 RGB LED helper functions

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/* It is recommended to copy this code in your example so that you can modify as
 * per your application's needs.
 */

#include <stdint.h>
#include <esp_err.h>
#include <esp_log.h>
#include "driver/gpio.h"

static const char *TAG = "valiturus_led";

#define VALITURUS_GPIO_PIN GPIO_NUM_19
#define HIGH 1
#define LOW 0

esp_err_t valiturus_led_init()
{
    gpio_pad_select_gpio(VALITURUS_GPIO_PIN);
    return gpio_set_direction(VALITURUS_GPIO_PIN, GPIO_MODE_OUTPUT);
}

esp_err_t valiturus_led_set_level(bool level)
{
    return gpio_set_level(VALITURUS_GPIO_PIN, level);
}