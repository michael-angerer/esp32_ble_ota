#include "nvs_flash.h"
#include "application/ota/ota.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_err.h"
#include "esp_pm.h"

// bluetooth part of the application
#include "application/bluetooth/bluetooth.h"

#define LOG_TAG_MAIN "main"

void app_main(void)
{
  ota_app_init();

  // enable light sleep
  esp_pm_config_esp32c3_t pm_config = {
      .max_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ, // e.g. 80, 160, 240
      .min_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ, // e.g. 40
      .light_sleep_enable = true,                      // enable light sleep
  };
  ESP_ERROR_CHECK(esp_pm_configure(&pm_config));

  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  bluetooth_init();
  /*
    TODO -

  */

  // !!!!!!!!!!!!!!!!!!!!
  // TODO
  // run diagnostics and rollback if image fails to update
  // rollback_image();
  // verify_image();

  vTaskDelay(200 * portTICK_PERIOD_MS);
  // ESP_LOGI(LOG_TAG_MAIN, "setting power");

  // // current pwoer
  // esp_power_level_t power = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_DEFAULT);

  // ESP_LOGI(LOG_TAG_MAIN, "power before: %d", power);

  // // set ble tx power to 6dm
  // esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N15);

  // vTaskDelay(100 * portTICK_PERIOD_MS);
  // power = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_DEFAULT);

  // ESP_LOGI(LOG_TAG_MAIN, "power before: %d", power);
}

/*

  NOTES:

    Increasing tx power to 21dBm causes the device to brownout. This is because it uses much more instaneous power and my 
    hardware is not good enough for that. :(

*/