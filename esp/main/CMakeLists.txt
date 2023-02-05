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

set(CPACK_SOURCE_IGNORE_FILES "examples/*")

set(MAIN_SRCS
    "main.c"
    # REMOVE EXAMPEL SOURCES AS GET LINKER CONFLICTS
    # "examples/lsm303agr_example/lsm303agr_example.c"
    # "examples/button_led_example/button_led_example.c"
    # "examples/ble_ota_example/ble_ota_example.c"
    # "examples/ble_ota_example/gatt_svr.c"
    # "examples/ble_ota_example/gap.c"
    # "examples/bmi_160_example/bmi_160_example.c"
    "application/exercise/exercise.c"
    "application/ota/ota.c"
    "application/bluetooth/bluetooth.c"
    "application/bluetooth/bluetooth_gatt.c"
    "application/bluetooth/bluetooth_gap.c"
    "application/bluetooth/bluetooth_ota.c"
    )

idf_component_register(SRCS ${MAIN_SRCS} 
                    INCLUDE_DIRS "application" REQUIRES ${MAIN_REQUIRES})