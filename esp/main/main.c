#include "nvs_flash.h"
#include "application/ota/ota.h"

// bluetooth part of the application
#include "application/bluetooth/bluetooth.h"

#define LOG_TAG_MAIN "main"

void app_main(void)
{
  ota_app_init();

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
}