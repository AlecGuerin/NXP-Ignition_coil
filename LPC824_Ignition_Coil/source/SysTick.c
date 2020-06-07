/*
 * SysTick.c
 *
 *	@brief Systick implementation.
 *  @date 29 févr. 2020
 *  @author Alec Guerin
 */


#include "SysTick.h"

volatile uint32_t _ticks = 0;

/**
 * @brief Initialize the Systick interruption to the provide [ms].
 * @param ms Millisecond to set.
 */
void SYSTICK_Init_ms(uint32_t ms)
{
	assert(CLOCK_GetFreq(kCLOCK_MainClk) > 1000);
	if(ms <1)
		ms= 1;

	SysTick_Config(ms * CLOCK_GetFreq(kCLOCK_MainClk)/1000);
}

/**
 * @brief Initialize the Systick interruption to the provide [us].
 * @param us Micro second to set.
 */
void SYSTICK_Init_us(uint32_t us)
{
	assert(CLOCK_GetFreq(kCLOCK_MainClk) > 1000000);
	if(us <1)
		us= 1;
	SysTick_Config(us * CLOCK_GetFreq(kCLOCK_MainClk)/1000000);
}

/**
 * @brief Systick event handler.
 *
 * Add one tick every time it's called.
 */
SysTick_Handler(void){
	_ticks++;
}

/**
 * @brief Get the current ticks.
 * @return Current ticks.
 */
uint32_t SYSTICK_GetTicks(void){
	return _ticks;
}
