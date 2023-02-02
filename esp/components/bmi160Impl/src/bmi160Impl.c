#include "bmi160Impl.h"

int8_t write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *read_data, uint16_t len)
{

    uint8_t writeData[1 + len];
    writeData[0] = reg_addr;
    for (int i = 1; i <= len; i++)
    {
        writeData[i] = *(read_data + i - 1);
    }

    esp_err_t ret;
    spi_transaction_t t = {
        .length = 8 + (len * 8),
        .flags = SPI_DEVICE_HALFDUPLEX,
        .tx_buffer = writeData,
        .user = (void *)0,
    };

    ESP_LOGI(BMI_TAG, "Sensor_IO_Write: spi_device_transmit");
    ret = spi_device_transmit(spi, &t);
    assert(ret == ESP_OK); // Should have had no issues.

    return 0;
}

int8_t read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{

    spi_device_acquire_bus(spi, portMAX_DELAY);

    spi_cmd(spi, reg_addr, true);

    uint8_t cmd = reg_addr;
    esp_err_t ret;
    spi_transaction_t t;

    memset(&t, 0, sizeof(t));
    t.length = 8 * len;
    // t.flags = SPI_TRANS_USE_RXDATA;
    t.user = (void *)1;
    t.rxlength = 8 * len;
    t.rx_buffer = data;

    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);

    spi_device_release_bus(spi);
    return 0;
}

void spi_cmd(spi_device_handle_t spi, uint8_t cmd, bool keep_cs_active)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t)); // Zero out the transaction
    t.length = 8;             // Command is 8 bits
    t.tx_buffer = &cmd;       // The data is the cmd itself
    t.user = (void *)0;       // D/C needs to be set to 0
    if (keep_cs_active)
    {
        t.flags = SPI_TRANS_CS_KEEP_ACTIVE; // Keep CS active after data transfer
    }
    ret = spi_device_polling_transmit(spi, &t); // Transmit!
    assert(ret == ESP_OK);                      // Should have had no issues.
}

void init_sensor_interface(void)
{
    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .miso_io_num = GPIO_NUM_4,
        .mosi_io_num = GPIO_NUM_5,
        .sclk_io_num = GPIO_NUM_6,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 1024};

    spi_device_interface_config_t devcfg = {

        .clock_speed_hz = 20000,    // Clock out at 10 MHz
        .mode = 0,                  // SPI mode 0
        .spics_io_num = GPIO_NUM_7, // CS pin
        .queue_size = 7,            // We want to be able to queue 7 transactions at a time

        // .pre_cb = lcd_spi_pre_transfer_callback, // Specify pre-transfer callback to handle D/C line

    };

    ESP_LOGI(BMI_TAG, "here");
    // Initialize the SPI bus
    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_DISABLED);
    ESP_ERROR_CHECK(ret);

    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}

void delay(uint32_t period)
{
    vTaskDelay(period / portTICK_PERIOD_MS);
}

void init_bmi160(void)
{
    int8_t rslt;

    rslt = bmi160_init(&bmi160dev);
    ESP_LOGI(BMI_TAG, "bmi160 init completw");

    if (rslt == BMI160_OK)
    {
        printf("BMI160 initialization success !\n");
        printf("Chip ID 0x%X\n", bmi160dev.chip_id);
    }
    else
    {
        printf("BMI160 initialization failure !\n");
        exit("fuck knoes what is going on here my Gs");
    }

    /* Select the Output data rate, range of accelerometer sensor */
    bmi160dev.accel_cfg.odr = BMI160_ACCEL_ODR_1600HZ;
    bmi160dev.accel_cfg.range = BMI160_ACCEL_RANGE_16G;
    bmi160dev.accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;

    /* Select the power mode of accelerometer sensor */
    bmi160dev.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

    /* Select the Output data rate, range of Gyroscope sensor */
    bmi160dev.gyro_cfg.odr = BMI160_GYRO_ODR_3200HZ;
    bmi160dev.gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
    bmi160dev.gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;

    /* Select the power mode of Gyroscope sensor */
    bmi160dev.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;

    /* Set the sensor configuration */
    rslt = bmi160_set_sens_conf(&bmi160dev);
}

void init_bmi160_sensor_driver_interface(void)
{

    /* SPI setup */

    /* link read/write/delay function of host system to appropriate
     *  bmi160 function call prototypes */
    bmi160dev.write = write;
    bmi160dev.read = read;
    bmi160dev.delay_ms = delay;
    bmi160dev.id = 209; // whatever the device id is
    bmi160dev.intf = BMI160_SPI_INTF;
}