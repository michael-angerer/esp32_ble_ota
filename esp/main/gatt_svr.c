#include "gatt_svr.h"

uint8_t gatt_svr_chr_ota_control_val;
uint8_t gatt_svr_chr_ota_packet_val[512];

uint16_t ota_control_val_handle;
uint16_t ota_packet_val_handle;

const esp_partition_t *update_partition;
esp_ota_handle_t update_handle;
bool updating = false;
uint16_t num_pkgs_received = 0;

static int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len,
                              uint16_t max_len, void *dst, uint16_t *len);
static int gatt_svr_chr_access_cb_read_write(uint16_t conn_handle,
                                             uint16_t attr_handle,
                                             struct ble_gatt_access_ctxt *ctxt,
                                             void *arg);

static int gatt_svr_chr_ota_control_cb(uint16_t conn_handle,
                                       uint16_t attr_handle,
                                       struct ble_gatt_access_ctxt *ctxt,
                                       void *arg);

static int gatt_svr_chr_ota_packet_cb(uint16_t conn_handle,
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
                    .access_cb = gatt_svr_chr_ota_control_cb,
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
                             BLE_GATT_CHR_F_NOTIFY,
                    .val_handle = &ota_control_val_handle,
                },
                {
                    // characteristic: OTA packet
                    .uuid = &gatt_svr_chr_ota_packet_uuid.u,
                    .access_cb = gatt_svr_chr_ota_packet_cb,
                    .flags = BLE_GATT_CHR_F_WRITE,
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

static void update_ota_control(uint16_t conn_handle) {
  struct os_mbuf *om;
  esp_err_t err;

  // check which value has been received
  switch (gatt_svr_chr_ota_control_val) {
    case SVR_CHR_OTA_CONTROL_REQUEST:
      // OTA request
      ESP_LOGI(LOG_TAG_GATT_SVR, "OTA has been requested via BLE.");
      // get the next free OTA partition
      update_partition = esp_ota_get_next_update_partition(NULL);
      // start the ota update
      err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES,
                          &update_handle);
      if (err != ESP_OK) {
        ESP_LOGE(LOG_TAG_GATT_SVR, "esp_ota_begin failed (%s)",
                 esp_err_to_name(err));
        esp_ota_abort(update_handle);
        gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_REQUEST_NAK;
      } else {
        gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_REQUEST_ACK;
      }

      // notify the client via BLE that the OTA has been acknowledged (or not)
      om = ble_hs_mbuf_from_flat(&gatt_svr_chr_ota_control_val,
                                 sizeof(gatt_svr_chr_ota_control_val));
      ble_gattc_notify_custom(conn_handle, ota_control_val_handle, om);
      ESP_LOGI(LOG_TAG_GATT_SVR, "OTA request acknowledgement has been sent.");

      updating = true;
      num_pkgs_received = 0;
      break;

    case SVR_CHR_OTA_CONTROL_DONE:

      updating = false;

      // end the OTA and start validation
      err = esp_ota_end(update_handle);
      if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
          ESP_LOGE(LOG_TAG_GATT_SVR,
                   "Image validation failed, image is corrupted!");
        } else {
          ESP_LOGE(LOG_TAG_GATT_SVR, "esp_ota_end failed (%s)!",
                   esp_err_to_name(err));
        }
      } else {
        // select the new partition for the next boot
        err = esp_ota_set_boot_partition(update_partition);
        if (err != ESP_OK) {
          ESP_LOGE(LOG_TAG_GATT_SVR, "esp_ota_set_boot_partition failed (%s)!",
                   esp_err_to_name(err));
        }
      }

      // set the control value
      if (err != ESP_OK) {
        gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_DONE_NAK;
      } else {
        gatt_svr_chr_ota_control_val = SVR_CHR_OTA_CONTROL_DONE_ACK;
      }

      // notify the client via BLE that DONE has been acknowledged
      om = ble_hs_mbuf_from_flat(&gatt_svr_chr_ota_control_val,
                                 sizeof(gatt_svr_chr_ota_control_val));
      ble_gattc_notify_custom(conn_handle, ota_control_val_handle, om);
      ESP_LOGI(LOG_TAG_GATT_SVR, "OTA DONE acknowledgement has been sent.");

      // restart the ESP to finish the OTA
      if (err == ESP_OK) {
        ESP_LOGI(LOG_TAG_GATT_SVR, "Preparing to restart!");
        vTaskDelay(pdMS_TO_TICKS(REBOOT_DEEP_SLEEP_TIMEOUT));
        esp_restart();
      }

      break;

    default:
      break;
  }
}

static int gatt_svr_chr_ota_control_cb(uint16_t conn_handle,
                                       uint16_t attr_handle,
                                       struct ble_gatt_access_ctxt *ctxt,
                                       void *arg) {
  int rc;
  uint8_t length = sizeof(gatt_svr_chr_ota_control_val);

  switch (ctxt->op) {
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

static int gatt_svr_chr_ota_packet_cb(uint16_t conn_handle,
                                      uint16_t attr_handle,
                                      struct ble_gatt_access_ctxt *ctxt,
                                      void *arg) {
  int rc;
  esp_err_t err;

  // store the received data into gatt_svr_chr_ota_packet_val
  rc = gatt_svr_chr_write(ctxt->om, 1, sizeof(gatt_svr_chr_ota_packet_val),
                          gatt_svr_chr_ota_packet_val, NULL);

  ESP_LOGE(LOG_TAG_GATT_SVR, "%x %x %x %x - %x", gatt_svr_chr_ota_packet_val[0],
           gatt_svr_chr_ota_packet_val[1], gatt_svr_chr_ota_packet_val[2],
           gatt_svr_chr_ota_packet_val[3], gatt_svr_chr_ota_packet_val[249]);

  // write the received packet to the partition
  if (updating) {
    err =
        esp_ota_write(update_handle, (const void *)gatt_svr_chr_ota_packet_val,
                      OTA_PACKET_SIZE);
    if (err != ESP_OK) {
      ESP_LOGE(LOG_TAG_GATT_SVR, "esp_ota_write failed (%s)!",
               esp_err_to_name(err));
    }
  }

  num_pkgs_received++;
  ESP_LOGI(LOG_TAG_GATT_SVR, "Received package %d", num_pkgs_received);

  return rc;
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

  if (ble_uuid_cmp(uuid, &gatt_svr_chr_ota_control_uuid.u) == 0) {
    data = &gatt_svr_chr_ota_control_val;
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
