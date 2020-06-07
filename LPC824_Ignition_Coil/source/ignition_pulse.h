/**
 *	@file ignition_pulse.h
 *
 *	@brief Contains functions to generate impulsions with the SC timer.
 *	@brief The pulse width precision is in [us]. The frequency precision is in [mHz].
 *  @date: 1 févr. 2020
 *  @author: Alec Guerin
 *
 *  Override PMW functions to set an impulsion width and a frequency.\n
 *  The precision for the pulse width is +- 1[us] and the frequency +- 1[mHz].\n
 *  These Methods are based on the 'SCTIMER_SetupPwm()' in drivers files fsl_sctimer.
 */

#ifndef IGNITION_PULSE_H_
#define IGNITION_PULSE_H_

#include "board.h"
#include "fsl_sctimer.h"

/**
 *@brief Initialize the impulsion width and frequency.
 *
 * @param base			SCTimer peripheral base address
 * @param pulseWidth_us	Pulse width in micro-second [us].
 * @param srcClock_Hz	SCTimer counter clock in Hertz [Hz].
 * @param freq_mHz		Pulse frequency in milli-Hertz [mHz].
 * @param output		Output to set.
 * @param event			Pointer to a variable where the pulse period event number is stored.
 *
 * @return	'kStatus_Success' on success.
 * @return	'kStatus_Fail' If the limit in terms of event count is reached or if an incorrect pulse/frequency values are provided.
 */
status_t IPULSE_SetupPulse(SCT_Type *base, uint32_t pulseWidth_us, uint32_t srcClock_Hz, uint32_t freq_mHz, sctimer_out_t output, uint32_t *event);

/**
 * @brief Updates the pulse frequency [mHz].
 *
 * @param base              SCTimer peripheral base address.
 * @param output            The output to configure.
 * @param srcClock_Hz		SCTimer counter clock in Hertz [Hz].
 * @param freq_mHz			Pulse frequency in milli-Hertz [mHz].
 * @param event             Pointer to a variable where the pulse period event number is stored.
 * @returns 'kStatus_Success' if succeed or 'kStatus_InvalidArgument'.
 */
status_t IPULSE_UpdatePulseFrequency(SCT_Type *base, sctimer_out_t output, uint32_t srcClock_Hz, uint32_t freq_mHz, uint32_t event);

/**
 * @brief Updates the pulse width [us].
 *
 * @param base              SCTimer peripheral base address.
 * @param output            The output to configure.
 * @param srcClock_Hz		Clock source in [Hz].
 * @param pulseWidth_us		Pulse width in micro-second[us].
 * @param event             Pointer to a variable where the pulse period event number is stored.
 * @returns 'kStatus_Success' if succeed or 'kStatus_InvalidArgument'.
 */
status_t IPULSE_UpdatePulseWidth(SCT_Type *base, sctimer_out_t output, uint32_t srcClock_Hz, uint32_t pulseWidth_us, uint32_t event);

/**
 * @brief Set the enable state of the pulsing.
 * @param base              SCTimer peripheral base address.
 * @param output            The output to configure.
 * @param enable			State to set.
 * @return 'kStatus_Success'.
 */
status_t IPULSE_EnablePulse(SCT_Type *base, sctimer_out_t output, uint8_t enable);

#endif /* IGNITION_PULSE_H_ */
