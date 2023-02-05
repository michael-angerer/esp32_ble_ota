#include "ota.h"
#include "esp_ota_ops.h"
#include "esp_log.h"

#define OTA_TAG "OTA"

/*
    This ota source file is responsible for hooking up ota to bluetooth
*/

int run_diagnostics()
{
    return 0;
}

/*
    ota_app_init is responsible for determining the running partition
*/
void ota_app_init()
{
    // check which partition is running
    const esp_partition_t *partition = esp_ota_get_running_partition();

    switch (partition->address)
    {
    case 0x00010000:
        ESP_LOGI(OTA_TAG, "Running partition: factory");
        break;
    case 0x00110000:
        ESP_LOGI(OTA_TAG, "Running partition: ota_0");
        break;
    case 0x00210000:
        ESP_LOGI(OTA_TAG, "Running partition: ota_1");
        break;
    default:
        ESP_LOGE(OTA_TAG, "Running partition: unknown");
        break;
    }

    // check if an OTA has been done, if so run diagnostics
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(partition, &ota_state) == ESP_OK)
    {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
        {
            ESP_LOGI(OTA_TAG, "An OTA update has been detected.");
            if (run_diagnostics())
            {
                ESP_LOGI(OTA_TAG,
                         "Diagnostics completed successfully! Continuing execution.");
                esp_ota_mark_app_valid_cancel_rollback();
            }
            else
            {
                ESP_LOGE(OTA_TAG,
                         "Diagnostics failed! Start rollback to the previous version.");
                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        }
    }
}

// If this is a valid image
void rollback_image()
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK)
    {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
        {

            ESP_LOGI(OTA_TAG, "Diagnostics completed successfully! Continuing execution ...");
            esp_ota_mark_app_valid_cancel_rollback();
        }
    }
    /*

        TODO - investigate if we should boot from here


    */
}

// verifies the image
void verify_image()
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK)
    {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
        {
            ESP_LOGE(OTA_TAG, "Diagnostics failed! Start rollback to the previous version ...");
            esp_ota_mark_app_invalid_rollback_and_reboot();
        }
    }
}
