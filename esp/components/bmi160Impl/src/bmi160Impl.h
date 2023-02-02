
#ifndef BMI160IMPL_H_
#define BMI160IMPL_H_

#ifdef __cplusplus
extern "C"
{
#endif

    // header here
    static const char *BMI_TAG = "BMI_IMPL";

#include <stdio.h>
#include "esp_system.h"
#include <string.h>
#include "esp_log.h"
#include "bmi160.h"
#include "bmi160_defs.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0                        /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_TX_BUF_DISABLE 0             /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0             /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000000

#define MPU9250_SENSOR_ADDR 0x19       /*!< Slave address of the MPU9250 sensor */
#define MPU9250_WHO_AM_I_REG_ADDR 0x75 /*!< Register addresses of the "who am I" register */

#define MPU9250_PWR_MGMT_1_REG_ADDR 0x6B /*!< Register addresses of the power managment register */
#define MPU9250_RESET_BIT 7

    spi_device_handle_t spi;
    /*! @brief This structure containing relevant bmi160 info */
    struct bmi160_dev bmi160dev;

    void spi_cmd(spi_device_handle_t spi, uint8_t cmd, bool keep_cs_active);

    /*! @brief variable to hold the bmi160 accel data */
    struct bmi160_sensor_data bmi160_accel;

    /*! @brief variable to hold the bmi160 gyro data */
    struct bmi160_sensor_data bmi160_gyro;

    /*!
     * @brief   internal API is used to initialize the sensor interface
     */
    void init_sensor_interface(void);

    /*!
     * @brief   This internal API is used to initialize the bmi160 sensor with default
     */
    void init_bmi160(void);

    /*!
     * @brief   This internal API is used to initialize the sensor driver interface
     */
    void init_bmi160_sensor_driver_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* BMI160_H_ */
