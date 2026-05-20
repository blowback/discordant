
#include "driver/usb_serial_jtag.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pf_all.h"
#include <stdio.h>
#include <unistd.h>

/* one-char pushback so KEY? can peek without losing the byte */
static int s_peeked = -1; /* -1 = empty, else the held byte */

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
  if (s_peeked >= 0) {
    int c = s_peeked;
    s_peeked = -1;
    return c;
  }

  uint8_t c;

  int n = usb_serial_jtag_read_bytes(&c, 1, portMAX_DELAY);
  return (n == 1) ? (int)c : -1;
}

cell_t sdQueryTerminal(void) {
  if (s_peeked >= 0) {
    return FTRUE; /* already have one buffered */
  }
  uint8_t c;
  int n = usb_serial_jtag_read_bytes(&c, 1, 0); /* non-blocking */
  if (n == 1) {
    s_peeked = c; /* hold it for the next sdTerminalIn */
    return FTRUE;
  }
  return FFALSE;
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
