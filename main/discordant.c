#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pforth.h"
#include "sdkconfig.h"
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

  ESP_LOGI("forth", "spawning forth task");
  configure_console();
  BaseType_t r = xTaskCreate(forth_task, "forth", 24576, NULL, 5, NULL);
  ESP_LOGI("forth", "xTaskCreate returned %d", (int)r);
}
