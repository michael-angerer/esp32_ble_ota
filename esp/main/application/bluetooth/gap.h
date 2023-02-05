
#ifndef BLUETOOTH_GAP_H_

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define LOG_TAG_GAP "gap"

    static const char device_name[] = "esp32";
    void advertise();
    void reset_cb(int reason);
    void sync_cb(void);
    void host_task(void *param);

#ifdef __cplusplus
}
#endif

#endif // !BLUETOOTH_GAP_H_