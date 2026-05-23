idf_component_register(
    SRCS "discordant.c"
    INCLUDE_DIRS "."
    REQUIRES pforth amy esp_driver_usb_serial_jtag
)
