#include "esp_log.h"
#include "driver/gpio.h"
#include "lsm303agrImpl.h"

/*
  Custom component includes
*/
#include "valiturus_button.h"
#include "valiturus_led.h"

#define LOG_TAG_LSM_EXAMPLE "LSM_EXAMPLE"

static int16_t data_raw_acceleration[3];
static int16_t data_raw_temperature;
static float acceleration_mg[3];
static float magnetic_mG[3];
static float temperature_degC;
static int16_t data_raw_magnetic[3];

/*
  sensor test app
*/
void sensor_test_app()
{
    // initalize the lsm chip
    uint8_t ret = lsmInit();
    if (ret != 0)
    {
        ESP_LOGI(LOG_TAG_LSM_EXAMPLE, "lsm init not succesfull, ret = %d", ret);
        return;
    }

    ESP_LOGI(LOG_TAG_LSM_EXAMPLE, "lsm init okay");

    while (1)
    {

        ret = lsmAccellDataReady();
        /*
          We are getting an error when we check the data is ready we get a data read lenth error.
          This causes us issues as when the error occurs the driver prints the error, which is blocking and the data wont be able to process as quickly as it has to wait for the debug printting.

          I think it occurs when lsmMagDataReady is called.

        */
        if (ret == 0 || ret == 1)
        {
            lsmReadAcellData(data_raw_acceleration, acceleration_mg);
            ESP_LOGI(LOG_TAG_LSM_EXAMPLE,
                     "acelleration [mg]:%4.2f\t%4.2f\t%4.2f\r\n",
                     acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);
        }
        else
        {
            ESP_LOGI(LOG_TAG_LSM_EXAMPLE, "lsm acelleration data not ready, ret = %d", ret);
        }

        ret = lsmMagDataReady();
        /*
          We are getting an error when we check the data is ready we get a data read lenth error.
          This causes us issues as when the error occurs the driver prints the error, which is blocking and the data wont be able to process as quickly as it has to wait for the debug printting.

          I think it occurs when lsmMagDataReady is called.

        */
        if (ret == 0 || ret == 1)
        {
            // Read acell data
            lsmReadMagData(data_raw_magnetic, magnetic_mG);
            ESP_LOGI(LOG_TAG_LSM_EXAMPLE,
                     "Magnetic field [mG]:%4.2f\t%4.2f\t%4.2f\r\n",
                     magnetic_mG[0], magnetic_mG[1], magnetic_mG[2]);
        }
        else
        {
            ESP_LOGI(LOG_TAG_LSM_EXAMPLE, "lsm acelleration data not ready, ret = %d", ret);
        }
    }
}