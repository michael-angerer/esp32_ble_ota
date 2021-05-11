#include "gatt_svr.h"

uint8_t gatt_svr_chr_ota_control_val[12];
uint8_t gatt_svr_chr_ota_packet_val[512];

uint16_t conn_handle;

uint16_t ota_control_val_handle;
uint16_t ota_packet_val_handle;

static int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len,
                              uint16_t max_len, void *dst, uint16_t *len);
static int gatt_svr_chr_access_cb_read_write(uint16_t conn_handle,
                                             uint16_t attr_handle,
                                             struct ble_gatt_access_ctxt *ctxt,
                                             void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        // service: OTA Service
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_ota_uuid.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    // characteristic: OTA control
                    .uuid = &gatt_svr_chr_ota_control_uuid.u,
                    .access_cb = gatt_svr_chr_access_cb_read_write,
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
                             BLE_GATT_CHR_F_NOTIFY,
                    .val_handle = &ota_control_val_handle,
                },
                {
                    // characteristic: OTA packet
                    .uuid = &gatt_svr_chr_ota_packet_uuid.u,
                    .access_cb = gatt_svr_chr_access_cb_read_write,
                    .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
                    .val_handle = &ota_packet_val_handle,
                },
                {
                    0,
                }},
    },

    {
        0,
    },
};

static int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len,
                              uint16_t max_len, void *dst, uint16_t *len) {
  uint16_t om_len;
  int rc;

  om_len = OS_MBUF_PKTLEN(om);
  if (om_len < min_len || om_len > max_len) {
    return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
  }

  rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
  if (rc != 0) {
    return BLE_ATT_ERR_UNLIKELY;
  }

  return 0;
}

static int gatt_svr_chr_access_cb_read_write(uint16_t conn_handle,
                                             uint16_t attr_handle,
                                             struct ble_gatt_access_ctxt *ctxt,
                                             void *arg) {
  int rc;
  void *data;
  uint16_t length;

  const ble_uuid_t *uuid = ctxt->chr->uuid;
  uint8_t write_access = ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR;
  uint8_t read_access = ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR;

  conn_handle = conn_handle;

  if (ble_uuid_cmp(uuid, &gatt_svr_chr_ota_control_uuid.u) == 0) {
    data = gatt_svr_chr_ota_control_val;
    length = 1;
  } else if (ble_uuid_cmp(uuid, &gatt_svr_chr_ota_packet_uuid.u) == 0) {
    data = gatt_svr_chr_ota_packet_val;
    length = 512;
  } else {
    ESP_LOGE(LOG_TAG_GATT_SVR, "Read/Write callback called with unknown uuid.");
    return BLE_ATT_ERR_UNLIKELY;
  }

  if (read_access) {
    rc = os_mbuf_append(ctxt->om, data, length);
    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  } else if (write_access) {
    rc = gatt_svr_chr_write(ctxt->om, 1, length, data, NULL);
    return rc;
  }

  assert(0);
  return BLE_ATT_ERR_UNLIKELY;
}

void gatt_svr_init() {
  ble_svc_gap_init();
  ble_svc_gatt_init();
  ble_gatts_count_cfg(gatt_svr_svcs);
  ble_gatts_add_svcs(gatt_svr_svcs);
}
