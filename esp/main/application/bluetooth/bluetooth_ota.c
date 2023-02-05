#include "bluetooth_gatt.h"
#include "freertos/FreeRTOS.h"
#include "esp_ota_ops.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "bluetooth_ota.h"
#include "bluetooth_gatt.h"

#define BLUETOOTH_OTA_TAG "Bluetooth OTA"

uint8_t gatt_svr_chr_ota_control_val;
uint8_t gatt_svr_chr_ota_data_val[512];

uint16_t ota_control_val_handle;
uint16_t ota_data_val_handle;
uint16_t ota_apply_update_val_handle;

const esp_partition_t *update_partition;
esp_ota_handle_t update_handle;
bool updating = false;

typedef struct
{
    uint16_t num_pkgs_received;
    uint16_t packet_size;
} ota_update_context;

ota_update_context ota_context = {
    .num_pkgs_received = 0,
    .packet_size = 0,
};

// if update is available and an update is requested by the client, set the new partition and reboot
// definitions for update app mechanisim
#define AVAILABLE true
#define UNAVAILABLE false

static bool updateAvailable = UNAVAILABLE;
static int app_partition_update_status = NONE_ATTEMPTED;

// updating the app partition should only be done if esp_ota is success
void update_app_partition(uint16_t conn_handle)
{
    // memory buffer for comms
    static struct os_mbuf *om;

    // there is no update available so the request is rubbish. return here
    if (!updateAvailable)
    {
        // notify the client that there has been no attempted update
        app_partition_update_status = NONE_ATTEMPTED;
        om = ble_hs_mbuf_from_flat(&app_partition_update_status,
                                   sizeof(app_partition_update_status));
        ble_gattc_notify_custom(conn_handle, ota_apply_update_val_handle, om);
        ESP_LOGI(LOG_TAG_GATT_SVR, "OTA no update attempted has been sent.");
        return;
    }

    // select the new partition for the next boot
    esp_err_t err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK)
    {
        // let the client know we have failed
        app_partition_update_status = UPDATE_FAIL;
        om = ble_hs_mbuf_from_flat(&app_partition_update_status,
                                   sizeof(app_partition_update_status));
        ble_gattc_notify_custom(conn_handle, ota_apply_update_val_handle, om);
        ESP_LOGE(LOG_TAG_GATT_SVR, "esp ota set boot partition failed (%s)!",
                 esp_err_to_name(err));
        return;
    }

    // Let the client that the update has been succesfull and then reboot
    app_partition_update_status = UPDATE_SUCCESS;
    om = ble_hs_mbuf_from_flat(&app_partition_update_status,
                               sizeof(app_partition_update_status));
    ble_gattc_notify_custom(conn_handle, ota_apply_update_val_handle, om);
    ESP_LOGI(LOG_TAG_GATT_SVR, "OTA update success, rebooting the sensor now <3");

    // restart the ESP to apply the OTA
    if (err == ESP_OK)
    {
        ESP_LOGI(LOG_TAG_GATT_SVR, "Preparing to restart!");
        vTaskDelay(portTICK_PERIOD_MS * 5);
        esp_restart();
    }
}

void update_ota_control(uint16_t conn_handle)
{
    static struct os_mbuf *om;
    static esp_err_t err;

    // check which value has been received
    switch (gatt_svr_chr_ota_control_val)
    {
    case SVR_CHR_OTA_CONTROL_REQUEST:

        // OTA request
        ESP_LOGI(BLUETOOTH_OTA_TAG, "OTA has been requested via BLE.");
        // get the next free OTA partition
        update_partition = esp_ota_get_next_update_partition(NULL);
        // start the ota update
        err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES,
                            &update_handle);
        if (err != ESP_OK)
        {
            ESP_LOGE(BLUETOOTH_OTA_TAG, "esp_ota_begin failed (%s)",
                     esp_err_to_name(err));
            esp_ota_abort(update_handle);
            gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_REQUEST_NAK;
        }
        else
        {
            gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_REQUEST_ACK;
            updating = true;

            // retrieve the packet size from OTA data
            ota_context.packet_size =
                (gatt_svr_chr_ota_data_val[1] << 8) + gatt_svr_chr_ota_data_val[0];
            ESP_LOGI(BLUETOOTH_OTA_TAG, "Packet size is: %d", ota_context.packet_size);

            ota_context.num_pkgs_received = 0;
        }

        // notify the client via BLE that the OTA has been acknowledged (or not)
        om = ble_hs_mbuf_from_flat(&gatt_svr_chr_ota_control_val,
                                   sizeof(gatt_svr_chr_ota_control_val));
        ble_gattc_notify_custom(conn_handle, ota_control_val_handle, om);
        ESP_LOGI(BLUETOOTH_OTA_TAG, "OTA request acknowledgement has been sent.");

        break;

    case SVR_CHR_OTA_CONTROL_DONE:

        updating = false;

        // end the OTA and start validation
        err = esp_ota_end(update_handle);
        if (err != ESP_OK)
        {
            if (err == ESP_ERR_OTA_VALIDATE_FAILED)
            {
                ESP_LOGE(BLUETOOTH_OTA_TAG,
                         "Image validation failed, image is corrupted!");
            }
            else
            {
                ESP_LOGE(BLUETOOTH_OTA_TAG, "esp_ota_end failed (%s)!",
                         esp_err_to_name(err));
            }
        }

        // set the control value
        if (err != ESP_OK)
        {
            gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_DONE_NAK;
        }
        else
        {
            gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_DONE_ACK;
            // set update available
            updateAvailable = true;
        }

        // notify the client via BLE that DONE has been acknowledged
        om = ble_hs_mbuf_from_flat(&gatt_svr_chr_ota_control_val,
                                   sizeof(gatt_svr_chr_ota_control_val));
        ble_gattc_notify_custom(conn_handle, ota_control_val_handle, om);
        ESP_LOGI(BLUETOOTH_OTA_TAG, "OTA DONE acknowledgement has been sent.");
        // When the client recieves this message, the client should send the request to set the new partition and reboot

        break;

    default:
        break;
    }
}

/*
  This callback that is called when OTA data charactaristic is requested for the OTA service.
  note: this function is static so all variables are initized only once and last for the lifetime of the application
*/
int gatt_svr_chr_ota_data_cb(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg)
{
    static int rc;
    static esp_err_t err;

    // store the received data into gatt_svr_chr_ota_data_val
    rc = gatt_svr_chr_write(ctxt->om, 1, sizeof(gatt_svr_chr_ota_data_val),
                            gatt_svr_chr_ota_data_val, NULL);

    // write the received packet to the partition
    if (updating)
    {
        err = esp_ota_write(update_handle, (const void *)gatt_svr_chr_ota_data_val,
                            ota_context.packet_size);
        if (err != ESP_OK)
        {
            ESP_LOGE(BLUETOOTH_OTA_TAG, "esp_ota_write failed (%s)!",
                     esp_err_to_name(err));
        }

        ota_context.num_pkgs_received++;
        ESP_LOGI(BLUETOOTH_OTA_TAG, "Received packet %d", ota_context.num_pkgs_received);
    }

    return rc;
}

/*
  This callback is applied when a user has applied the
*/
int gatt_svr_chr_ota_apply_update(uint16_t conn_handle, uint16_t attr_handle,
                                  struct ble_gatt_access_ctxt *ctxt,
                                  void *arg)
{

    ESP_LOGI(LOG_TAG_GATT_SVR, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

    // todo - return and handle error
    update_app_partition(conn_handle);
    return 0;
}

/*
  This callback that is called when OTA control charactaristic is requested for the OTA service.
  note: this function is static so all variables are initized only once and last for the lifetime of the application
*/
int gatt_svr_chr_ota_control_cb(uint16_t conn_handle,
                                uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt,
                                void *arg)
{
    static int rc;
    static int8_t length = sizeof(gatt_svr_chr_ota_control_val);

    // Here is where we set the control value based on the request from the client, so we should do something similar!
    switch (ctxt->op)
    {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        // a client is reading the current value of ota control
        rc = os_mbuf_append(ctxt->om, &gatt_svr_chr_ota_control_val, length);
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        break;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        // a client is writing a value to ota control
        rc = gatt_svr_chr_write(ctxt->om, 1, length,
                                &gatt_svr_chr_ota_control_val, NULL);
        // update the OTA state with the new value
        update_ota_control(conn_handle);
        return rc;
        break;

    default:
        break;
    }

    // this shouldn't happen
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}