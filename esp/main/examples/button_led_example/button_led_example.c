
#include "button_led_example.h"
#include "esp_ota_ops.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/*
  Custom component includes
*/
#include "valiturus_button.h"
#include "valiturus_led.h"

#define BUTTON_LED_EXAMPLE_TAG "BUTTON_LED_EXAMPLE"

int tickCount = 0;

/*
  button interrupt handler
*/
static void IRAM_ATTR button_interrupt_handler(void *args)
{
    tickCount++;
}

// LED_Control_Task
void LED_Control_Task(void *params)
{
    bool level = false;

    while (1)
    {
        vTaskDelay(portTICK_PERIOD_MS * 10);
        ESP_LOGI(BUTTON_LED_EXAMPLE_TAG, "New app with blink");
        ESP_LOGI(BUTTON_LED_EXAMPLE_TAG, "tick count %d", tickCount);

        level = !level;
        valiturus_led_set_level(level);
    }
}

void button_led_test_app()
{
    ESP_LOGI(BUTTON_LED_EXAMPLE_TAG, "initing LED");
    gpio_reset_pin(GPIO_NUM_19);
    valiturus_led_init();

    ESP_LOGI(BUTTON_LED_EXAMPLE_TAG, "init button");
    gpio_reset_pin(GPIO_NUM_18);
    valiturus_button_init(button_interrupt_handler);

    // init tasks
    xTaskCreate(LED_Control_Task, "LED_Control_Task", 2048, NULL, 1, NULL);
}