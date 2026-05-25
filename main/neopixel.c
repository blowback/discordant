
/********************************************************************************
 * @file neopixel.c
 * @copyright ant.org, 2026
 * @author Ant Skelton
 *
 * @brief NeoPixel LED driver.
 *
 * The board has a single WS2812B "NeoPixel" RGB LED which we can use for
 * status etc.
 ********************************************************************************/

/*============================================================================*
 ANSI C & System-wide Header Files
 *============================================================================*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "led_strip_types.h"
#include "led_strip.h"

/*============================================================================*
 Interface Header Files
 *============================================================================*/
/* None */

/*============================================================================*
 Local Header File
 *============================================================================*/
#include "neopixel.h"

/*============================================================================*
 Public Data
 *============================================================================*/
/* None */

/*============================================================================*
 Private Defines
 *============================================================================*/
/* None */

/*============================================================================*
 Private Data Types
 *============================================================================*/
/* None */

/*============================================================================*
 Private Function Prototypes
 *============================================================================*/
static void lights_out_cb(TimerHandle_t h);

/*============================================================================*
 Private Data
 *============================================================================*/
static const char *tag = "neopixel";
static led_strip_handle_t led_strip = NULL;
static TimerHandle_t lights_out = NULL;

/*============================================================================*
 Public Function Implementations
 *============================================================================*/

/********************************************************************************
 * @Brief Initialise the NeoPixel LED.
 *
 * Allocate driver resources and turn on LED power.
 *
 * @return Nothing.
 ********************************************************************************/
void neopixel_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = NEOPIXEL_DATA,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10 MHz
        .flags.with_dma = true,
    };

    gpio_set_direction(NEOPIXEL_POWER, GPIO_MODE_OUTPUT);
    gpio_set_level(NEOPIXEL_POWER, 1);

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    lights_out = xTimerCreate("LIGHTSOOT", pdMS_TO_TICKS(1000), pdFALSE, NULL, &lights_out_cb);
}

/********************************************************************************
 * @Brief Shutdown NeoPixel LED.
 *
 * Release driver resources and disable the power to the LED.
 *
 * @return Nothing.
 ********************************************************************************/
void neopixel_deinit(void)
{
    if(led_strip) {
        led_strip_del(led_strip);
        led_strip = NULL;
    }
    gpio_set_level(NEOPIXEL_POWER, 0);
}

/********************************************************************************
 * @Brief Set NeoPixel to given RGB colour.
 *
 * @param red Red component 0..255
 * @param green Green component 0..255
 * @param blue Blue component 0..255
 *
 * @return Nothing.
 ********************************************************************************/
void neopixel_rgb(uint32_t red, uint32_t green, uint32_t blue)
{
    if(led_strip) {
        led_strip_set_pixel(led_strip, 0, red, green, blue);
        led_strip_refresh(led_strip);
    }
}

/********************************************************************************
 * @Brief Set NeoPixel to given HSV colour.
 *
 * @param hue Hue component 0..359
 * @param saturation Saturation component 0..255
 * @param value Value component 0..255
 *
 * @return Nothing.
 ********************************************************************************/
void neopixel_hsv(uint16_t hue, uint8_t saturation, uint8_t value)
{
    if(led_strip) {
        led_strip_set_pixel_hsv(led_strip, 0, hue, saturation, value);
        led_strip_refresh(led_strip);
    }
}

/********************************************************************************
 * @Brief Turn off the NeoPixel LED.
 *
 * @return Nothing.
 ********************************************************************************/
void neopixel_clear(void)
{
    if(led_strip) {
        led_strip_clear(led_strip);
    }
}

/********************************************************************************
 * @Brief Set a timer to douse the NeoPixel LED after a short while.
 *
 * After the given number of milliseconds the NeoPixel LED will be extinguished.
 *
 * @param ms Number of milliseconds to wait before turning LED off.
 *
 * @return Nothing.
 ********************************************************************************/
void neopixel_lights_out_in_ms(uint32_t ms)
{
    xTimerChangePeriod(lights_out, pdMS_TO_TICKS(ms), 0);
}

/********************************************************************************
 * @Brief Blah blah blah..
 *
 * Blah blah blah blah blah.
 *
 * @param a Blah blah.
 * @param[out] b Blah blah.
 *
 * @return Blah.
 *
 ********************************************************************************/
void neopixel_cancel_lights_out(void)
{
    if(xTimerIsTimerActive(lights_out)) {
        xTimerStop(lights_out, 0);
    }
}

/*============================================================================*
 Private Functions
 *============================================================================*/
/********************************************************************************
 * @Brief Handle the "lights_out" timer event.
 *
 * Turns the NeoPixel LED off.
 *
 * @param h Timer handle.
 *
 * @return Nothing.
 ********************************************************************************/
static void lights_out_cb(TimerHandle_t h)
{
    neopixel_clear();
}

/*============================================================================*
 End Of File
 *============================================================================*/
