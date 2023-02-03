#ifndef LSM303AGRIMPl_H_
/*

    Custom implementation of lsm303agr library for my specific platform

*/

#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "lsm303agr_reg.h"
#include "esp_log.h"

// cpp gaurd
#ifdef __cplusplus
extern "C"
{
#endif
#define I2C_ACK_VAL 0x0
#define I2C_NACK_VAL 0x1

    static const char *LSM_IMPL_TAG = "LSM IMPL";

    int i2c_write_slave(uint8_t bus, uint8_t addr, const uint8_t *reg,
                        uint8_t *data, uint32_t len);
    int32_t i2c_read_slave(i2c_port_t port, uint8_t addr, const uint8_t *reg,
                           uint8_t *data, uint32_t len);

    int32_t platform_write_accell(void *handle, uint8_t Reg, const uint8_t *Bufp, uint16_t len);
    int32_t platform_read_accell(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len);
    int32_t platform_write_mag(void *handle, uint8_t Reg, const uint8_t *Bufp, uint16_t len);
    int32_t platform_read_mag(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len);

    void platform_delay(uint32_t ms);
    void platform_init(void);

    // `private` functions
    int __lsmAcellInit();
    int __lsmMagInit();
    int __checkMinimumArraySize(void *array, uint8_t minimumLength);

    // high level implementations
    // similar to be implemented by bmi 160 library
    //  then we can create a higher level library
    int lsmInit();
    int lsmAccellDataReady();
    int lsmMagDataReady();
    int lsmReadMagData(int16_t *data_raw_acceleration, float *acceleration_mg);
    int lsmReadAcellData(int16_t *data_raw_acceleration, float *acceleration_mg);
    int lsmReadTempratureData();

#ifdef __cplusplus
}
#endif

#endif // !LSM303AGRIMPl_H_