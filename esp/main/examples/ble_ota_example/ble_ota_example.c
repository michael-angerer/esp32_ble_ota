#include "esp_log.h"
#include "esp_ota_ops.h"
#include "gap.h"
#include "gatt_svr.h"
#include "nvs_flash.h"
#include "esp_bt.h"

/*
  A GATT service is a collection of charactarstics.
  For example: a heart rate service contains a heart rate measurement charactaristic and a a body location charactaristic.

  What does GATT do?
    - defines the format of services and ther characteristics.
    - defines the procedures that are used to interface with theese atributes such as service discovery,
      characteristic reads, characteristic writes, notifications and indications.
        
*/

#define BLE_OTA_EXAMPLE_TAG "BLE_OTA_EXAMPLE"

int run_diagnostics()
{
  return 0;
}

void ble_ota_example()
{
  // check which partition is running
  const esp_partition_t *partition = esp_ota_get_running_partition();

  switch (partition->address)
  {
  case 0x00010000:
    ESP_LOGI(BLE_OTA_EXAMPLE_TAG, "Running partition: factory");
    break;
  case 0x00110000:
    ESP_LOGI(BLE_OTA_EXAMPLE_TAG, "Running partition: ota_0");
    break;
  case 0x00210000:
    ESP_LOGI(BLE_OTA_EXAMPLE_TAG, "Running partition: ota_1");
    break;
  default:
    ESP_LOGE(BLE_OTA_EXAMPLE_TAG, "Running partition: unknown");
    break;
  }

  // check if an OTA has been done, if so run diagnostics
  esp_ota_img_states_t ota_state;
  if (esp_ota_get_state_partition(partition, &ota_state) == ESP_OK)
  {
    if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
    {
      ESP_LOGI(BLE_OTA_EXAMPLE_TAG, "An OTA update has been detected.");
      if (run_diagnostics())
      {
        ESP_LOGI(BLE_OTA_EXAMPLE_TAG,
                 "Diagnostics completed successfully! Continuing execution.");
        esp_ota_mark_app_valid_cancel_rollback();
      }
      else
      {
        ESP_LOGE(BLE_OTA_EXAMPLE_TAG,
                 "Diagnostics failed! Start rollback to the previous version.");
        esp_ota_mark_app_invalid_rollback_and_reboot();
      }
    }
  }

  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // BLE Setup
  esp_bt_mem_release(ESP_BT_MODE_BTDM);

  // initialize BLE controller and nimble stack
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
