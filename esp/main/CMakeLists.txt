set(MAIN_REQUIRES
    driver
    valiturus_led
    valiturus_button
    nvs_flash
    bt
    bootloader
    app_update
    BMI160_driver
    bmi160Impl
    lsm303agr-pid
    lsm303agrImpl
)

set(MAIN_SRCS
    "main.c"
    "examples/lsm303agr_example/lsm303agr_example.c"
    "examples/button_led_example/button_led_example.c"
    "examples/ble_ota_example/ble_ota_example.c"
    "examples/ble_ota_example/gatt_svr.c"
    "examples/ble_ota_example/gap.c"
    "examples/bmi_160_example/bmi_160_example.c"
    "application/bluetooth/exercise/exercise.c"
    "application/bluetooth/ota/ota.c"
    "application/bluetooth/bluetooth.c"
    "application/bluetooth/bluetooth_gatt.c"
    "application/bluetooth/bluetooth_gap.c"
    )

idf_component_register(SRCS ${MAIN_SRCS} 
                    INCLUDE_DIRS "." REQUIRES ${MAIN_REQUIRES})