/**
 *  @file led.c
 *
 *  @date Created on: 1 Feb. 2020
 *  @author Alec Guerin
 */

#include "led.h"
#include "fsl_gpio.h"


volatile uint8_t _ledState = 0x00;	///< Static value used to keep a previous LED state.

/**
 * @brief Get the LED status.
 * The status is compose of a byte. Green LED is in position '0' and
 * red LED is in position '1'.
 * @return The LEDs status.
 */
uint8_t LED_GetState(){
	return _ledState;
}

/**
 * @brief Set the LED status.
 * The status is compose of a byte. Green LED is in position '0' and
 * red LED is in position '1'.
 * @param state LEDs state to set.
 */
void LED_SetState(uint8_t state){
	state &= 0x03;
	_ledState = state;
	// Manage green LED
	GPIO_PinWrite(GPIO, 0, LED_GREEN_LED, state & 1 << LED_POS_GREEN);
	// Manage red LED
	GPIO_PinWrite(GPIO, 0, LED_RED_LED, state & 1 << LED_POS_RED);
}

/**
 * @brief Reset all LEDs.
 */
void LED_ResetAll(void){

	_ledState = 0x00;
	// Reset green LED
	GPIO_PinWrite(GPIO, 0, LED_GREEN_LED, 0);
	// Reset green LED
	GPIO_PinWrite(GPIO, 0, LED_RED_LED, 0);
}

/**
 * @brief Set all LEDs.
 */
void LED_SetAll(void){

	_ledState = 0x03;
	// Set green LED
	GPIO_PinWrite(GPIO, 0, LED_GREEN_LED, 1);
	// Set green LED
	GPIO_PinWrite(GPIO, 0, LED_RED_LED, 1);
}

/**
 * @brief Set the provided LED.
 *
 * @param ledPin Pin LED to set.
 * @param state State of the LED.
 */
void LED_SetLed(uint8_t ledPin, uint8_t state){

	_ledState = LED_UpdateBit(_ledState, ledPin == LED_POS_GREEN,state);
	_ledState = LED_UpdateBit(_ledState, ledPin ==LED_POS_RED,state);

	// Set the LED
	GPIO_PinWrite(GPIO, 0, ledPin, state);

}

/**
 * @brief Set or reset a bit in provided 'value' at position 'pos'.
 * @param value Initial value.
 * @param pos	bit position.
 * @param set	Set or Reset bit.
 * @return Updated value.
 */
uint8_t LED_UpdateBit(uint8_t value, uint8_t pos, uint8_t set){

	if(set){
		SET_BIT(value, pos);
	}else{
		RESET_BIT(value, pos);
	}
	return value;
}
