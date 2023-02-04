#include "esp_log.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "driver/gpio.h"
#include "bmi160.h"
#include "bmi160Impl.h"
#include "lsm303agrImpl.h"
#include "esp_system.h"

#define BMI_160_EXAMPLE_TAG "BMI_160_EXAMPLE"

void bmi_160_example()
{

  // bmi160 impl example

  int times_to_read = 0;

  // initialize function pointers etc for spi driver
  init_bmi160_sensor_driver_interface();

  // initialize my spi device
  init_sensor_interface();

  init_bmi160();

  while (times_to_read < 100)
  {
    // To read both Accel and Gyro data
    bmi160_get_sensor_data((BMI160_ACCEL_SEL | BMI160_GYRO_SEL), &bmi160_accel, &bmi160_gyro, &bmi160dev); // this should cause a linker error

    ESP_LOGI(BMI_160_EXAMPLE_TAG, "ax:%d\tay:%d\taz:%d\n", bmi160_accel.x, bmi160_accel.y, bmi160_accel.z);
    ESP_LOGI(BMI_160_EXAMPLE_TAG, "gx:%d\tgy:%d\tgz:%d\n", bmi160_gyro.x, bmi160_gyro.y, bmi160_gyro.z);

    vTaskDelay(250 / portTICK_PERIOD_MS);
    times_to_read = times_to_read + 1;
  }
}
