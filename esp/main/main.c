#include "esp_log.h"
#include "esp_ota_ops.h"
#include "gap.h"
#include "gatt_svr.h"
#include "nvs_flash.h"

#define LOG_TAG_MAIN "main"

void app_main(void) {
  const esp_app_desc_t *app_desc = esp_ota_get_app_description();
  ESP_LOGI(LOG_TAG_MAIN, "App version: %s", app_desc->version);

  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // BLE Setup

  // initialize BLE controller and nimble stack
  esp_nimble_hci_and_controller_init();
  nimble_port_init();

  // register sync and reset callbacks
  ble_hs_cfg.sync_cb = sync_cb;
  ble_hs_cfg.reset_cb = reset_cb;

  // initialize service table
  gatt_svr_init();

  // set device name and start host task
  ble_svc_gap_device_name_set(device_name);
  nimble_port_freertos_init(host_task);
}