#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c_slave.h"
#include "amy.h"

static const char *TAG = "i2c_task";

#define I2C_SLAVE_POWER  7          /* drive this HIGH to tunr on the port */
#define I2C_SLAVE_SCL_IO 4          /* gpio number for i2c slave clock */
#define I2C_SLAVE_SDA_IO 3          /* gpio number for i2c slave data */
#define I2C_SLAVE_NUM    0
#define ESP_SLAVE_ADDR   0x2a       /* ESP slave address (wr = 0x54 r = 0x55 )*/

// Command Lists
#define STARS_COMMAND          (0x10)
#define FORKS_COMMAND          (0x20)
#define OPENISSUES_COMMAND     (0x30)
#define DESCRIPTIONS_COMMAND   (0x40)

// ---- frame types on the I²C wire ----
#define FRAME_MIDI3   0x00   // [0x00][status][d1][d2]
#define FRAME_MIDI2   0x01   // [0x01][status][d1]
#define FRAME_WIRE    0x02   // [0x02][len][AMY wire message bytes...]
#define FRAME_PING    0x03   // [0x03]

typedef struct {
    uint8_t tmp_buffer_stars[sizeof(int)];
    uint8_t tmp_buffer_forks[sizeof(int)];
    uint8_t tmp_buffer_open_issues[sizeof(int)];
    uint8_t tmp_buffer_descriptions[100];
    QueueHandle_t event_queue;
    uint8_t command_data;
    i2c_slave_dev_handle_t handle;
} i2c_slave_context_t;

typedef enum {
    I2C_SLAVE_EVT_RX,
    I2C_SLAVE_EVT_TX
} i2c_slave_event_type_t;

typedef struct {
    i2c_slave_event_type_t type;
    unsigned char *buf;
    size_t len;
} i2c_slave_event_t;

void handle_frame(const uint8_t *buf, size_t len) {
    ESP_LOGI(TAG, "Received %d byte message", len);
    switch (buf[0]) {

        case FRAME_MIDI3:
        case FRAME_MIDI2: {
            uint8_t status = buf[1];
            uint8_t hi = status & 0xF0;
            uint8_t ch = status & 0x0F;          // 0..15

            if (hi == 0x90 && buf[3] > 0) {       // note-on, velocity > 0
                amy_event e = amy_default_event();
                e.synth      = ch + 1;            // AMY synths are 1..16
                e.midi_note  = buf[2];
                e.velocity   = buf[3] / 127.0f;
                amy_add_event(&e);
            }
            else if (hi == 0x80 || (hi == 0x90 && buf[3] == 0)) {  // note-off
                amy_event e = amy_default_event();
                e.synth     = ch + 1;
                e.midi_note = buf[2];
                e.velocity  = 0;                  // velocity 0 = note off
                amy_add_event(&e);
            }
            else if (hi == 0xC0) {                // program change
                amy_event e = amy_default_event();
                e.synth        = ch + 1;
                e.patch_number = buf[2];          // 0-127 Juno, +128 for DX7 bank
                amy_add_event(&e);
            }
            else if (hi == 0xE0) {                // pitch bend
                int bend = ((buf[2] << 7) | buf[1]) - 8192;
                amy_event e = amy_default_event();
                e.pitch_bend = bend / 8192.0f;    // AMY: octaves, ± 
                amy_add_event(&e);
            }
            else if (hi == 0xB0) {                // control change
                // CC handling — see note below
            }
            break;
        }

        case FRAME_WIRE: {
            // payload IS an AMY wire message — and it's already NUL terminated
            amy_add_message((char *)&buf[2]);
            break;
        }

        case FRAME_PING:
            // queue a status byte for the next I²C read; assert INT
            break;
    }
}

// MASTER requests data - ISR context
static bool i2c_slave_request_cb(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_request_event_data_t *evt_data, void *arg)
{
    i2c_slave_context_t *context = (i2c_slave_context_t *)arg;
    i2c_slave_event_t evt = { .type = I2C_SLAVE_EVT_TX };
    BaseType_t xTaskWoken = 0;
    xQueueSendFromISR(context->event_queue, &evt, &xTaskWoken);
    return xTaskWoken;
}

// MASTER has sent data - ISR context
static bool i2c_slave_receive_cb(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_rx_done_event_data_t *evt_data, void *arg)
{
    i2c_slave_context_t *context = (i2c_slave_context_t *)arg;
    BaseType_t xTaskWoken = 0;
    unsigned char *ptr = evt_data->buffer;
    size_t len = evt_data->length;

    // allocate one extra byte so we can NUL terminate whatever was sent
    unsigned char *buf = (unsigned char *)malloc(len + 1);

    if(buf) {
        memcpy(buf, ptr, len);
        buf[len] = 0;

        i2c_slave_event_t evt = {
            .type = I2C_SLAVE_EVT_RX,
            .buf = buf,
            .len = len, // excluding final NUL
        };

        xQueueSendFromISR(context->event_queue, &evt, &xTaskWoken);
    } else {
        ESP_LOGE(TAG, "Can't alloc i2c command buffer");
    }
    return xTaskWoken;
}

static void i2c_slave_task(void *arg)
{
    i2c_slave_context_t *context = (i2c_slave_context_t *)arg;
    //i2c_slave_dev_handle_t handle = (i2c_slave_dev_handle_t)context->handle;

    //uint8_t zero_buffer[32] = {}; // Use this buffer to clear the fifo.
    //uint32_t write_len, total_written;
    //uint32_t buffer_size = 0;
    bool exit = false;

    while (!exit) {
        i2c_slave_event_t evt;

        if (xQueueReceive(context->event_queue, &evt, 10) == pdTRUE) {
            switch(evt.type) {
                case I2C_SLAVE_EVT_TX:
                    // MASTER requests data

                    /* total_written = 0; */
                    /**/
                    /* while (total_written < buffer_size) { */
                    /*     ESP_ERROR_CHECK(i2c_slave_write(handle, data_buffer + total_written, buffer_size - total_written, &write_len, 1000)); */
                    /**/
                    /*     if (write_len == 0) { */
                    /*         ESP_LOGE(TAG, "Write error or timeout"); */
                    /*         exit = true; */
                    /*     } */
                    /**/
                    /*     total_written += write_len; */
                    /* } */
                    break;

                case I2C_SLAVE_EVT_RX:
                    // MASTER sent data
                    handle_frame(evt.buf, evt.len);
                    free(evt.buf);
                    evt.buf = NULL;
                    evt.len = 0;
                    break;
            }
        }
    }
    vTaskDelete(NULL);
}

void i2c_setup(void)
{
    static i2c_slave_context_t context = {0};

    ESP_LOGI(TAG, "Setting up i2c slave");

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << I2C_SLAVE_POWER),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(I2C_SLAVE_POWER, 1);

    context.event_queue = xQueueCreate(16, sizeof(i2c_slave_event_t));

    if (!context.event_queue) {
        ESP_LOGE(TAG, "Creating queue failed");
        return;
    }

    i2c_slave_config_t i2c_slv_config = {
        .i2c_port = I2C_SLAVE_NUM,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .scl_io_num = I2C_SLAVE_SCL_IO,
        .sda_io_num = I2C_SLAVE_SDA_IO,
        .slave_addr = ESP_SLAVE_ADDR,
        .send_buf_depth = 100,
        .receive_buf_depth = 100,
    };

    ESP_ERROR_CHECK(i2c_new_slave_device(&i2c_slv_config, &context.handle));

    i2c_slave_event_callbacks_t cbs = {
        .on_receive = i2c_slave_receive_cb,
        .on_request = i2c_slave_request_cb,
    };

    ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(context.handle, &cbs, &context));

    xTaskCreate(i2c_slave_task, "i2c_slave_task", 1024 * 4, &context, 10, NULL);
}
