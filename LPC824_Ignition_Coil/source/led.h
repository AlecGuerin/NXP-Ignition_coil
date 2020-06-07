/**
 * @file led.h
 *
 *  @date Created on: 1 Feb. 2020
 *  @author AG
 */

#ifndef LED_H_
#define LED_H_

#include "pin_mux.h"
#include "fsl_gpio.h"

#define SET_BIT(value, pos) value |= 1 << pos		///< Set bit to '1' at the provided index.
#define RESET_BIT(value, pos) value &= ~(1 << pos)	///< Set bit to '0' at the provided index.

#define LED_GREEN_LED  BOARD_INITPINS_LED1_PIN		///< Green LED pin GPIO position.
#define LED_RED_LED  BOARD_INITPINS_LED2_PIN		///< Red LED pin GPIO position.

#define LED_POS_GREEN 0		///< Green LED bit index position.
#define LED_POS_RED 1		///< Red LED bit index position.


uint8_t LED_GetState();				///< Get the LED status.
void LED_SetState(uint8_t state);	///< Set the LED status.

void LED_ResetAll(void);			///< Reset all LEDs.
void LED_SetAll(void);				///< Set all LEDs.

void LED_SetLed(uint8_t ledPin, uint8_t state);		///< Set the provide LED to the provide state.

uint8_t LED_UpdateBit(uint8_t value, uint8_t pos, uint8_t set);	///< Update a bit to the provided value.

#endif
