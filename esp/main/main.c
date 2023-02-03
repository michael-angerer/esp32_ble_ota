#include "examples/lsm303agr_example/lsm303agr_example.h"
#include "examples/button_led_example/button_led_example.h"
#include "examples/ble_ota_example/ble_ota_example.h"
#include "examples/bmi_160_example/bmi_160_example.h"

#define LOG_TAG_MAIN "main"

void app_main(void)
{
  bmi_160_example();
}