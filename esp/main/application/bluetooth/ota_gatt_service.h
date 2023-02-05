#pragma once

#ifndef OTA_GATT_SERVICE_H_
#ifdef __cplusplus
extern "C"
{
#endif
    // callbacks used by thing 
    
    int gatt_svr_chr_ota_apply_update(uint16_t conn_handle, uint16_t attr_handle,
                                      struct ble_gatt_access_ctxt *ctxt,
                                      void *arg);

    int gatt_svr_chr_ota_data_cb(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt,
                                 void *arg);

    int gatt_svr_chr_ota_control_cb(uint16_t conn_handle,
                                    uint16_t attr_handle,
                                    struct ble_gatt_access_ctxt *ctxt,
                                    void *arg);

#ifdef __cplusplus
}
#endif

#endif // !OTA_GATT_SERVICE_H_
