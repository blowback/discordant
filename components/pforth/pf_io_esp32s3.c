#if 0
/* pForth terminal I/O for ESP32-S3 over UART0 */
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pf_all.h"

#define PF_UART UART_NUM_0
cell_t sdQueryTerminal(void) {
  /* non-blocking: is at least one char available? */
  size_t avail = 0;
  uart_get_buffered_data_len(PF_UART, &avail);
  return (avail > 0) ? FTRUE : FFALSE;
}

int sdTerminalOut(char c) {
  uart_write_bytes(PF_UART, &c, 1);
  return 0;
}

int sdTerminalEcho(char c) {
  /* pForth's line editor calls this to echo typed chars */
  return sdTerminalOut(c);
}

int sdTerminalFlush(void) {
  uart_wait_tx_done(PF_UART, portMAX_DELAY);
  return 0;
}

int sdTerminalIn(void) {
  uint8_t c;
  /* blocking single-byte read */
  int n = uart_read_bytes(PF_UART, &c, 1, portMAX_DELAY);
  return (n == 1) ? (int)c : -1;
}

void sdTerminalInit(void) {
  /* UART driver is installed in app_main before the Forth task starts,
     so nothing to do here. */
}

void sdTerminalTerm(void) {}

cell_t sdSleepMillis(cell_t msec) {
  vTaskDelay(pdMS_TO_TICKS(msec));
  return 0;
}
#endif

#include "driver/usb_serial_jtag.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pf_all.h"
#include <stdio.h>
#include <unistd.h>

#if 0
int sdTerminalOut(char c) {
  putchar(c);
  return 0;
}

int sdTerminalEcho(char c) { return sdTerminalOut(c); }

int sdTerminalFlush(void) {
  fflush(stdout);
  return 0;
}

int sdTerminalIn(void) {
  int c = getchar(); /* see note below about blocking */
  return (c == EOF) ? -1 : c;
}

cell_t sdQueryTerminal(void) {
  return FALSE; /* simple: always "no char waiting" */
}
#endif
int sdTerminalOut(char c) {
  usb_serial_jtag_write_bytes((const uint8_t *)&c, 1, portMAX_DELAY);
  return 0;
}

int sdTerminalEcho(char c) { return sdTerminalOut(c); }

int sdTerminalFlush(void) {
  /* driver writes synchronously above; nothing buffered */
  return 0;
}

int sdTerminalIn(void) {
  uint8_t c;
  /* genuine blocking read — portMAX_DELAY waits until a byte arrives */
  int n = usb_serial_jtag_read_bytes(&c, 1, portMAX_DELAY);
  return (n == 1) ? (int)c : -1;
}

cell_t sdQueryTerminal(void) {
  uint8_t c;
  /* non-blocking peek: 0 timeout */
  int n = usb_serial_jtag_read_bytes(&c, 1, 0);
  /* note: this consumes the byte — see caveat below */
  return (n == 1) ? FTRUE : FFALSE;
}

void sdTerminalInit(void) {}

void sdTerminalTerm(void) {}

cell_t sdSleepMillis(cell_t ms) {
  vTaskDelay(pdMS_TO_TICKS(ms));
  return 0;
}

/* pForth calls this when resizing a file; we have no filesystem,
   so just report failure. Signature must match pf_io.h. */
ThrowCode sdResizeFile(FileStream *stream, uint64_t size) {
  TOUCH(stream);
  TOUCH(size);
  return -1; /* non-zero = error */
}
