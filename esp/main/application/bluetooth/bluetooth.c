#include "esp_log.h"
#include "bluetooth_gap.h"
#include "bluetooth_gatt.h"
#include "esp_bt.h"

#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

/*
  A GATT service is a collection of charactarstics.
  For example: a heart rate service contains a heart rate measurement charactaristic and a a body location charactaristic.

  What does GATT do?
    - defines the format of services and ther characteristics.
    - defines the procedures that are used to interface with theese atributes such as service discovery,
      characteristic reads, characteristic writes, notifications and indications.

*/

#define BLE_OTA_EXAMPLE_TAG "BLE_OTA_EXAMPLE"


void bluetooth_init()
{
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