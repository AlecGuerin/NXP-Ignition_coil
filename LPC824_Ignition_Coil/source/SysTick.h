/*
 * SysTick.h
 *	@brief Systick header.
 *  @date 29 févr. 2020
 *  @author Alec Guerin
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "fsl_common.h"

/**
 * @brief Initialize the Systick interruption to the provide [ms].
 * @param ms Millisecond to set.
 */
void SYSTICK_Init_ms(uint32_t ms);

/**
 * @brief Initialize the Systick interruption to the provide [us].
 * @param us Micro second to set.
 */
void SYSTICK_Init_us(uint32_t us);

/**
 * @brief Handler of the systick event.
 * Add one tick.
 */
SysTick_Handler(void);

/**
 * @brief Get the current ticks.
 * @return Current ticks.
 */
uint32_t SYSTICK_GetTicks(void);

#endif /* SYSTICK_H_ */
