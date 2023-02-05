#pragma once

#include "host/ble_uuid.h"

#define LOG_TAG_GATT_SVR "gatt_svr"
#define REBOOT_DEEP_SLEEP_TIMEOUT 500
#define GATT_DEVICE_INFO_UUID 0x180A
#define GATT_MANUFACTURER_NAME_UUID 0x2A29
#define GATT_MODEL_NUMBER_UUID 0x2A24

typedef enum
{
    SVR_CHR_OTA_CONTROL_NOP,
    SVR_CHR_OTA_CONTROL_REQUEST,
    SVR_CHR_OTA_CONTROL_REQUEST_ACK,
    SVR_CHR_OTA_CONTROL_REQUEST_NAK,
    SVR_CHR_OTA_CONTROL_DONE,
    SVR_CHR_OTA_CONTROL_DONE_ACK,
    SVR_CHR_OTA_CONTROL_DONE_NAK,
} svr_chr_ota_control_val_t;

typedef enum
{
    UPDATE_SUCCESS,
    UPDATE_FAIL,
    NONE_ATTEMPTED,
} svr_chr_apply_update_val_t;

// service: OTA Service
// d6f1d96d-594c-4c53-b1c6-244a1dfde6d8
static const ble_uuid128_t gatt_svr_svc_ota_uuid =
    BLE_UUID128_INIT(0xd8, 0xe6, 0xfd, 0x1d, 0x4a, 024, 0xc6, 0xb1, 0x53, 0x4c,
                     0x4c, 0x59, 0x6d, 0xd9, 0xf1, 0xd6);

// characteristic: OTA Control
// 7ad671aa-21c0-46a4-b722-270e3ae3d830
static const ble_uuid128_t gatt_svr_chr_ota_control_uuid =
    BLE_UUID128_INIT(0x30, 0xd8, 0xe3, 0x3a, 0x0e, 0x27, 0x22, 0xb7, 0xa4, 0x46,
                     0xc0, 0x21, 0xaa, 0x71, 0xd6, 0x7a);

// characteristic: OTA Data
// 23408888-1f40-4cd8-9b89-ca8d45f8a5b0
static const ble_uuid128_t gatt_svr_chr_ota_data_uuid =
    BLE_UUID128_INIT(0xb0, 0xa5, 0xf8, 0x45, 0x8d, 0xca, 0x89, 0x9b, 0xd8, 0x4c,
                     0x40, 0x1f, 0x88, 0x88, 0x40, 0x23);

// characteristic: OTA Data
// 3e33db7b-9108-4549-b063-979f55610f0f
// static const ble_uuid128_t gatt_svr_chr_ota_apply_update_uuid =
//     BLE_UUID128_INIT(0x0f, 0x0f, 0x61, 0x55, 0x9f, 0x97, 0x63, 0xb0, 0x49, 0x45,
//                      0x08, 0x91, 0x7b, 0xdb, 0x33, 0x3e);

// 3e000000-0000-0000-0000-000000000000
static const ble_uuid128_t gatt_svr_chr_ota_apply_update_uuid =
    BLE_UUID128_INIT(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x3e);

void gatt_svr_init();

int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len,
                       uint16_t max_len, void *dst, uint16_t *len);