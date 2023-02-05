#include "bluetooth_gatt.h"
#include "freertos/FreeRTOS.h"
#include "esp_ota_ops.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "bluetooth_ota.h"
/*
    Application code that implements bluetooth functionality
*/

/*
  A GATT service is a collection of charactarstics.
  For example: a heart rate service contains a heart rate measurement charactaristic and a a body location charactaristic.

  What does GATT do?
    - defines the format of services and ther characteristics.
    - defines the procedures that are used to interface with theese atributes such as service discovery,
      characteristic reads, characteristic writes, notifications and indications.

*/

static const char *manuf_name = "Valiturus SOT";
static const char *model_num = "ESP32";

static int gatt_svr_chr_access_device_info(uint16_t conn_handle,
                                           uint16_t attr_handle,
                                           struct ble_gatt_access_ctxt *ctxt,
                                           void *arg);

// Handles defined in bluetooth_ota source
extern uint16_t ota_control_val_handle;
extern uint16_t ota_data_val_handle;
extern uint16_t ota_apply_update_val_handle;

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {// Service: Device Information
     .type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
     .characteristics =
         (struct ble_gatt_chr_def[]){
             {
                 // Characteristic: Manufacturer Name
                 .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),
                 .access_cb = gatt_svr_chr_access_device_info,
                 .flags = BLE_GATT_CHR_F_READ,
             },
             {
                 // Characteristic: Model Number
                 .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),
                 .access_cb = gatt_svr_chr_access_device_info,
                 .flags = BLE_GATT_CHR_F_READ,
             },
             {
                 0,
             },
         }},

    {
        // service: OTA Service
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_ota_uuid.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    // characteristic: OTA control
                    .access_cb = gatt_svr_chr_ota_control_cb,
                    .uuid = &gatt_svr_chr_ota_control_uuid.u,
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
                             BLE_GATT_CHR_F_NOTIFY,
                    .val_handle = &ota_control_val_handle,
                },
                {
                    // characteristic: OTA data
                    .uuid = &gatt_svr_chr_ota_data_uuid.u,
                    .access_cb = gatt_svr_chr_ota_data_cb,
                    .flags = BLE_GATT_CHR_F_WRITE,
                    .val_handle = &ota_data_val_handle,
                },
                {
                    // characteristic: OTA set partition and reboot
                    .uuid = &gatt_svr_chr_ota_apply_update_uuid.u,
                    .access_cb = gatt_svr_chr_ota_apply_update,
                    .flags =
                        BLE_GATT_CHR_F_WRITE |
                        BLE_GATT_CHR_PROP_NOTIFY |
                        BLE_GATT_CHR_F_INDICATE |
                        BLE_GATT_CHR_F_NOTIFY,
                    .val_handle = &ota_apply_update_val_handle,
                },
                {
                    0,
                }},
    },

    {
        0,
    },
};

static int gatt_svr_chr_access_device_info(uint16_t conn_handle,
                                           uint16_t attr_handle,
                                           struct ble_gatt_access_ctxt *ctxt,
                                           void *arg)
{
    uint16_t uuid;
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    if (uuid == GATT_MODEL_NUMBER_UUID)
    {
        rc = os_mbuf_append(ctxt->om, model_num, strlen(model_num));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    if (uuid == GATT_MANUFACTURER_NAME_UUID)
    {
        rc = os_mbuf_append(ctxt->om, manuf_name, strlen(manuf_name));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len,
                       uint16_t max_len, void *dst, uint16_t *len)
{
    static uint16_t om_len;
    static int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len)
    {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0)
    {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

void gatt_svr_init()
{
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svr_svcs);
    ble_gatts_add_svcs(gatt_svr_svcs);
}
