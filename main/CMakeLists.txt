idf_component_register(
    SRCS "discordant.c" "i2c.c" "neopixel.c"
    INCLUDE_DIRS "."
    REQUIRES pforth amy esp_driver_usb_serial_jtag esp_driver_i2c esp_driver_gpio led_strip
)
