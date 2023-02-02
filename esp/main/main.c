#include "esp_log.h"
#include "esp_ota_ops.h"
#include "gap.h"
#include "gatt_svr.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "driver/gpio.h"

/*
  Custom component includes
*/
#include "valiturus_button.h"
#include "valiturus_led.h"

#define LOG_TAG_MAIN "main"

int tickCount = 0;

/*
  button interrupt handler
*/
static void IRAM_ATTR button_interrupt_handler(void *args)
{
  tickCount++;
}

bool run_diagnostics()
{
  // do some diagnostics
  return true;
}

// LED_Control_Task
void LED_Control_Task(void *params)
{
  bool level = false;

  while (1)
  {
    vTaskDelay(portTICK_PERIOD_MS * 100);
    ESP_LOGI(LOG_TAG_MAIN, "New app with blink");
    ESP_LOGI(LOG_TAG_MAIN, "tick count %d", tickCount);

    level = !level;
    valiturus_led_set_level(level);
  }
}

void app_main(void)
{
  // check which partition is running
  const esp_partition_t *partition = esp_ota_get_running_partition();

  switch (partition->address)
  {
  case 0x00010000:
    ESP_LOGI(LOG_TAG_MAIN, "Running partition: factory");
    break;
  case 0x00110000:
    ESP_LOGI(LOG_TAG_MAIN, "Running partition: ota_0");
    break;
  case 0x00210000:
    ESP_LOGI(LOG_TAG_MAIN, "Running partition: ota_1");
    break;

  default:
    ESP_LOGE(LOG_TAG_MAIN, "Running partition: unknown");
    break;
  }

  // check if an OTA has been done, if so run diagnostics
  esp_ota_img_states_t ota_state;
  if (esp_ota_get_state_partition(partition, &ota_state) == ESP_OK)
  {
    if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
    {
      ESP_LOGI(LOG_TAG_MAIN, "An OTA update has been detected.");
      if (run_diagnostics())
      {
        ESP_LOGI(LOG_TAG_MAIN,
                 "Diagnostics completed successfully! Continuing execution.");
        esp_ota_mark_app_valid_cancel_rollback();
      }
      else
      {
        ESP_LOGE(LOG_TAG_MAIN,
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

  ESP_LOGI(LOG_TAG_MAIN, "initing LED");
  gpio_reset_pin(GPIO_NUM_19);
  valiturus_led_init();

  ESP_LOGI(LOG_TAG_MAIN, "init button");
  gpio_reset_pin(GPIO_NUM_18);
  valiturus_button_init(button_interrupt_handler);

  // init tasks
  xTaskCreate(LED_Control_Task, "LED_Control_Task", 2048, NULL, 1, NULL);
}
