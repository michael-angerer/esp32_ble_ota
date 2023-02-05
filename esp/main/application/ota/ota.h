
#ifndef OTA_UPDATES_H_

#ifdef __cplusplus
extern "C"
{
#endif
    void rollback_image();
    void ota_app_init();
    void verify_image();
#ifdef __cplusplus
}
#endif

#endif // !OTA_UPDATES_H_