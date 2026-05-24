#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pforth.h"
#include "amy.h"
#include "sdkconfig.h"


#include "i2c.h"

#define PF_UART UART_NUM_0

static void forth_task(void *arg) {
    /* args: dictionary filename, source filename, flags.
   NULL, NULL, 0 -> no .dic file: cold-start from C primitives only.
   This is the equivalent of pforth's -i mode. */
    ESP_LOGI("forth", "forth_task entered, calling pfDoForth");
    pfDoForth(NULL, NULL, 0);
    ESP_LOGI("forth", "pfDoForth returned");
    vTaskDelete(NULL); /* if Forth ever exits */
}

#if 0
#include "driver/uart.h"
#include "driver/uart_vfs.h"

static void configure_console(void) {
    if (!uart_is_driver_installed((uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM)) {
        ESP_ERROR_CHECK(uart_driver_install(
            (uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0));
    }
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    uart_vfs_dev_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);
    uart_vfs_dev_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM,
                                          ESP_LINE_ENDINGS_CR);
    uart_vfs_dev_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM,
                                          ESP_LINE_ENDINGS_CRLF);
}
#endif

static void configure_console(void) {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    usb_serial_jtag_driver_config_t cfg = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&cfg));

    usb_serial_jtag_vfs_use_driver();
    usb_serial_jtag_vfs_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    usb_serial_jtag_vfs_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);
}

void app_main(void) {

    amy_config_t amy_cfg = amy_default_config();
    
    amy_cfg.platform.multithread = 1;
    amy_cfg.platform.multicore = 1;

    amy_cfg.features.startup_bleep = 1;
    amy_cfg.features.default_synths = 0;
    amy_cfg.features.audio_in = 0;

    amy_cfg.midi = AMY_MIDI_IS_NONE;

    amy_cfg.audio = AMY_AUDIO_IS_I2S;
    amy_cfg.i2s_mclk = -1; // PCM5102A self clocks
    amy_cfg.i2s_bclk = 16;
    amy_cfg.i2s_lrc = 18;
    amy_cfg.i2s_dout = 17;

    amy_start(amy_cfg);
    i2c_setup();

    ESP_LOGI("forth", "spawning forth task");
    configure_console();
    BaseType_t r = xTaskCreate(forth_task, "forth", 24576, NULL, 5, NULL);
    ESP_LOGI("forth", "xTaskCreate returned %d", (int)r);

    #if 0
    // note on
    amy_event on = amy_default_event();
    on.synth     = 1;
    on.midi_note = 60;
    on.velocity  = 1.0f;
    amy_add_event(&on);

    amy_event on2 = amy_default_event();
    on2.synth     = 1;
    on2.midi_note = 64;
    on2.velocity  = 1.0f;
    amy_add_event(&on2);

    amy_event on3 = amy_default_event();
    on3.synth     = 1;
    on3.midi_note = 67;
    on3.velocity  = 1.0f;
    amy_add_event(&on3);

    vTaskDelay(pdMS_TO_TICKS(1000));   // hold for 1 s

    // note off
    amy_event off = amy_default_event();
    off.synth     = 1;
    off.midi_note = 60;
    off.velocity  = 0;                 // velocity 0 = note off
    amy_add_event(&off);
    amy_event off2 = amy_default_event();
    off2.synth     = 1;
    off2.midi_note = 64;
    off2.velocity  = 0;                 // velocity 0 = note off2
    amy_add_event(&off2);

    amy_event off3 = amy_default_event();
    off3.synth     = 1;
    off3.midi_note = 67;
    off3.velocity  = 0;                 // velocity 0 = note off3
    amy_add_event(&off3);
    #endif
}
