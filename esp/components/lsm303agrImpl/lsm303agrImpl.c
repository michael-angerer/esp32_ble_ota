#include "lsm303agrImpl.h"

/*

    Platform specific functions

*/

#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0                        /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_TX_BUF_DISABLE 0             /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0             /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000000

#define MPU9250_PWR_MGMT_1_REG_ADDR 0x6B /*!< Register addresses of the power managment register */
#define MPU9250_RESET_BIT 7

int32_t platform_write_accell(void *handle, uint8_t Reg, const uint8_t *Bufp, uint16_t len)
{
    esp_err_t ret = i2c_write_slave(I2C_MASTER_NUM, 0x32, &Reg, (uint8_t *)Bufp, len);
    return ret;
}

int32_t platform_read_accell(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len)
{
    esp_err_t ret = i2c_read_slave(I2C_MASTER_NUM, 0x32, &Reg, Bufp, len);
    return ret;
}

int32_t platform_write_mag(void *handle, uint8_t Reg, const uint8_t *Bufp, uint16_t len)
{
    esp_err_t ret = i2c_write_slave(I2C_MASTER_NUM, 0x3C, &Reg, (uint8_t *)Bufp, len);
    return ret;
}

int32_t platform_read_mag(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len)
{
    esp_err_t ret = i2c_read_slave(I2C_MASTER_NUM, 0x3C, &Reg, Bufp, len);
    return ret;
}

int32_t i2c_read_slave(i2c_port_t port, uint8_t addr, const uint8_t *reg,
                       uint8_t *data, uint32_t len)
{
    if (len == 0)
        return true;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (reg)
    {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, addr | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, *reg | 0x80, true);

        if (!data)
            i2c_master_stop(cmd);
    }
    if (data)
    {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, addr | I2C_MASTER_READ, true);
        i2c_master_read(cmd, data, len - 1, (i2c_ack_type_t)I2C_ACK_VAL);
        i2c_master_read_byte(cmd, data + len - 1, (i2c_ack_type_t)I2C_NACK_VAL);
        i2c_master_stop(cmd);
    }
    esp_err_t err = i2c_master_cmd_begin(port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return err;
}

int i2c_write_slave(uint8_t bus, uint8_t addr, const uint8_t *reg,
                    uint8_t *data, uint32_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, addr | I2C_MASTER_WRITE, true);
    if (reg)
        i2c_master_write_byte(cmd, *reg | 0x80, true);
    if (data)
        i2c_master_write(cmd, data, len, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(bus, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return err;
}

void platform_delay(uint32_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void platform_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    // init i2c device TODO
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_0,
        .scl_io_num = GPIO_NUM_1,
        .sda_pullup_en = GPIO_PULLDOWN_DISABLE,
        .scl_pullup_en = GPIO_PULLDOWN_DISABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(i2c_master_port, &conf);

    i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

/*

High Level sensor functions

*/

/*

    Platform implementation for lsm sensor library


*/
uint8_t lsm_mag_rst = 0;
stmdev_ctx_t dev_ctx_xl = {};
stmdev_ctx_t dev_ctx_mg = {};
lsm303agr_reg_t reg = {};

// initalize magnetometre
int lsmInit()
{

    dev_ctx_xl.write_reg = platform_write_accell;
    dev_ctx_xl.read_reg = platform_read_accell;

    dev_ctx_mg.write_reg = platform_write_mag;
    dev_ctx_mg.read_reg = platform_read_mag;

    platform_init();

    // Wait sensor boot time
    platform_delay(20);

    // initialize magnetometre
    // uint8_t ret = __lsmMagInit(dev_ctx_mg);
    // if (ret != 0)
    // {    //     return ret;
    // }

    // initialize accelleration device
    uint8_t ret = __lsmAcellInit();
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

// initialize lsm303 magnetometre
int __lsmMagInit()
{
    // Check mag device address
    uint8_t whoamI = 0;
    lsm303agr_mag_device_id_get(&dev_ctx_mg, &whoamI);
    if (whoamI != LSM303AGR_ID_MG)
    {
        return -1;
    }

    /* Restore default configuration for magnetometer */
    esp_err_t ret = lsm303agr_mag_reset_set(&dev_ctx_mg, PROPERTY_ENABLE);
    if (ret != 0)
    {
        return ret;
    }

    do
    {
        lsm303agr_mag_reset_get(&dev_ctx_mg, &lsm_mag_rst);
    } while (lsm_mag_rst);

    ret = lsm303agr_mag_block_data_update_set(&dev_ctx_mg, PROPERTY_ENABLE);
    if (ret != 0)
    {
        return ret;
    }

    /* Set Output Data Rate */
    ret = lsm303agr_mag_data_rate_set(&dev_ctx_mg, LSM303AGR_MG_ODR_10Hz);
    if (ret != 0)
    {
        return ret;
    }
    /* Set accelerometer full scale */
    /* Set / Reset magnetic sensor mode */
    ret = lsm303agr_mag_set_rst_mode_set(&dev_ctx_mg,
                                         LSM303AGR_SENS_OFF_CANC_EVERY_ODR);
    if (ret != 0)
    {
        return ret;
    }
    /* Enable temperature compensation on mag sensor */
    ret = lsm303agr_mag_offset_temp_comp_set(&dev_ctx_mg, PROPERTY_ENABLE);

    /* Enable temperature sensor */
    /* Set device in continuous mode */
    /* Set magnetometer in continuous mode */
    ret = lsm303agr_mag_operating_mode_set(&dev_ctx_mg,
                                           LSM303AGR_CONTINUOUS_MODE);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int __lsmAcellInit()
{
    // Check accell device address
    uint8_t whoamI = 0;
    lsm303agr_xl_device_id_get(&dev_ctx_xl, &whoamI);
    if (whoamI != LSM303AGR_ID_XL)
    {
        return -1;
    }

    /* Enable Block Data Update */
    esp_err_t ret = lsm303agr_xl_block_data_update_set(&dev_ctx_xl, PROPERTY_ENABLE);
    if (ret != 0)
    {
        return ret;
    }

    /* Set Output Data Rate */
    ret = lsm303agr_xl_data_rate_set(&dev_ctx_xl, LSM303AGR_XL_ODR_100Hz);
    if (ret != 0)
    {
        return ret;
    }

    /* Set accelerometer full scale */
    ret = lsm303agr_xl_full_scale_set(&dev_ctx_xl, LSM303AGR_2g);
    if (ret != 0)
    {
        return ret;
    }

    /* Enable temperature sensor */
    ret = lsm303agr_temperature_meas_set(&dev_ctx_xl, LSM303AGR_TEMP_ENABLE);
    if (ret != 0)
    {
        return ret;
    }

    /* Set device in continuous mode */
    ret = lsm303agr_xl_operating_mode_set(&dev_ctx_xl, LSM303AGR_HR_12bit);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

int lsmAccellDataReady()
{
    uint8_t ret = lsm303agr_xl_status_get(&dev_ctx_xl, &reg.status_reg_a);
    if (ret != 0)
    {
        return -1;
    }
    return reg.status_reg_a.zyxda;
}

int lsmMagDataReady()
{
    uint8_t ret = lsm303agr_mag_status_get(&dev_ctx_mg, &reg.status_reg_m);
    if (ret != 0)
    {
        return -1;
    }
    return reg.status_reg_m.zyxda;
}

int lsmReadAcellData(int16_t *data_raw_acceleration, float *acceleration_mg)
{

    // check raw accelleration has enough size
    uint8_t ret = __checkMinimumArraySize(data_raw_acceleration, 3);
    if (ret != 0)
    {
        return -1;
    }

    ret = __checkMinimumArraySize(acceleration_mg, 3);
    if (ret != 0)
    {
        return -1;
    }

    // reset raw data memory
    memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
    ret = lsm303agr_acceleration_raw_get(&dev_ctx_xl, data_raw_acceleration);
    if (ret != 0)
    {
        return -1;
    }

    acceleration_mg[0] = lsm303agr_from_fs_2g_hr_to_mg(
        data_raw_acceleration[0]);
    acceleration_mg[1] = lsm303agr_from_fs_2g_hr_to_mg(
        data_raw_acceleration[1]);
    acceleration_mg[2] = lsm303agr_from_fs_2g_hr_to_mg(
        data_raw_acceleration[2]);

    return 0;
}

int lsmReadMagData(int16_t *data_raw_magnetic, float *magnetic_mG)
{

    // check raw accelleration has enough size
    uint8_t ret = __checkMinimumArraySize(data_raw_magnetic, 3);
    if (ret != 0)
    {
        return -1;
    }

    ret = __checkMinimumArraySize(magnetic_mG, 3);
    if (ret != 0)
    {
        return -1;
    }

    // reset raw data memory
    memset(data_raw_magnetic, 0x00, 3 * sizeof(int16_t));
    ret = lsm303agr_acceleration_raw_get(&dev_ctx_xl, data_raw_magnetic);
    if (ret != 0)
    {
        return -1;
    }

    magnetic_mG[0] = lsm303agr_from_lsb_to_mgauss(
        data_raw_magnetic[0]);
    magnetic_mG[1] = lsm303agr_from_lsb_to_mgauss(
        data_raw_magnetic[1]);
    magnetic_mG[2] = lsm303agr_from_lsb_to_mgauss(
        data_raw_magnetic[2]);

    return 0;
}

// checkMinimumArraySize checks that the array is atleast minimumLength items long
int __checkMinimumArraySize(void *array, uint8_t minimumLength)
{
    if (array == NULL)
    {
        return -1;
    }

    // ignoring length check for now :(
    // size_t dataArrayLength = sizeof((int16_t *) array) / sizeof((int16_t) 0);
    // if (dataArrayLength < minimumLength)
    // {    //     return -1;
    // }
    return 0;
}