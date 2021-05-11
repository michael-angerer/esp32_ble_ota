#include "esp_log.h"
#include "esp_ota_ops.h"

#define LOG_TAG_MAIN "main"

void app_main(void) {

    const esp_app_desc_t *app_desc = esp_ota_get_app_description();
    ESP_LOGI(LOG_TAG_MAIN, "App version: %s", app_desc->version);

}