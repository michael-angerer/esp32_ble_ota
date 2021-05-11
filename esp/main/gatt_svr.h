#pragma once

#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define LOG_TAG_GATT_SVR "gatt_svr"

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

// characteristic: OTA Packet
// 23408888-1f40-4cd8-9b89-ca8d45f8a5b0
static const ble_uuid128_t gatt_svr_chr_ota_packet_uuid =
    BLE_UUID128_INIT(0xb0, 0xa5, 0xf8, 0x45, 0x8d, 0xca, 0x89, 0x9b, 0xd8, 0x4c,
                     0x40, 0x1f, 0x88, 0x88, 0x40, 0x23);

void gatt_svr_init();