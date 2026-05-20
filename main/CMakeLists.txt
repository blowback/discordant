idf_component_register(
    SRCS "discordant.c"
    INCLUDE_DIRS "."
    REQUIRES pforth esp_driver_usb_serial_jtag
)
