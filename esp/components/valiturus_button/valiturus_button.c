/*  WS2812 RGB LED helper functions

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/* It is recommended to copy this code in your example so that you can modify as
 * per your application's needs.
 */

#include "valiturus_button.h"
#include "esp_log.h"

static const char *TAG = "valiturus button";

#define VALITURUS_BUTTON_PIN GPIO_NUM_18

// esp_err_t valiturus_button_init(onButtonPress routine)
esp_err_t valiturus_button_init(onButtonPress routine)
{
    // set gpio mode
    gpio_config_t pinConf = {
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
        .pin_bit_mask = 1 << VALITURUS_BUTTON_PIN,
    };

    gpio_config(&pinConf);

    gpio_install_isr_service(0); // add edge triggered interrupt service
    return gpio_isr_handler_add(VALITURUS_BUTTON_PIN, routine, NULL);
}