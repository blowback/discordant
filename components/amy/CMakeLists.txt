file(GLOB AMY_SOURCES "upstream/src/*.c")

list(REMOVE_ITEM AMY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/upstream/src/amy-example.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/upstream/src/libminiaudio-audio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/upstream/src/examples.c"

)

idf_component_register(
    SRCS ${AMY_SOURCES}
    INCLUDE_DIRS "upstream/src"
    REQUIRES
        esp_driver_uart      # driver/uart.h   — MIDI UART
        esp_driver_i2s       # driver/i2s_*.h  — audio output
        esp_driver_gpio      # driver/gpio.h
        esp_timer            # esp_timer.h
        esp_driver_i2c       # driver/i2c_*.h  — if AMY touches I²C
)
