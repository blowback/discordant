
/********************************************************************************
 * @file neopixel.h
 * @copyright ant.org, 2026
 * @author Ant Skelton
 *
 * @brief Blah blah blah.
 *
 * Blah blah blah blah blah.
 ********************************************************************************/

#ifndef INC_NEOPIXEL_H
#define INC_NEOPIXEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/*============================================================================*
 Public Defines
 *============================================================================*/

// GPIOs
#define NEOPIXEL_DATA 33
#define NEOPIXEL_POWER 21

/*============================================================================*
 Public Data Types
 *============================================================================*/
/* None */

/*============================================================================*
 Public Data
 *============================================================================*/
/* None */

/*============================================================================*
 Public Functions
 *============================================================================*/
void neopixel_init(void);
void neopixel_deinit(void);
void neopixel_rgb(uint32_t red, uint32_t green, uint32_t blue);
void neopixel_hsv(uint16_t hue, uint8_t saturation, uint8_t value);
void neopixel_clear(void);
void neopixel_lights_out_in_ms(uint32_t ms);
void neopixel_cancel_lights_out(void);

#ifdef __cplusplus
}
#endif

#endif /* ndef INC_NEOPIXEL_H */

/*============================================================================*
 End Of File
 *============================================================================*/
