set(MAIN_REQUIRES
    valiturus_led
    valiturus_button
    nvs_flash
    bt
    bootloader
    app_update
)

idf_component_register(SRCS "gap.c" "gatt_svr.c" "main.c" 
                    INCLUDE_DIRS "." REQUIRES ${MAIN_REQUIRES})