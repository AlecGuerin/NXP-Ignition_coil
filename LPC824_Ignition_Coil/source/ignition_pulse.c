/**
 *	@file ignition_pulse.c
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

#include "ignition_pulse.h"

static uint32_t s_currentEvent;		///< Keep track of SCTimer event number.


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
status_t IPULSE_SetupPulse(SCT_Type *base,
						  uint32_t pulseWidth_us,
						  uint32_t srcClock_Hz,
                          uint32_t freq_mHz,
						  sctimer_out_t output,
                          uint32_t *event)
{
    assert(pulseWidth_us);
    assert(srcClock_Hz);
    assert(freq_mHz);
    assert(output < FSL_FEATURE_SCT_NUMBER_OF_OUTPUTS);

    uint32_t period, pulsePeriod = 0;
    uint32_t sctClock    = srcClock_Hz / (((base->CTRL & SCT_CTRL_PRE_L_MASK) >> SCT_CTRL_PRE_L_SHIFT) + 1);
    uint32_t periodEvent = 0, pulseEvent = 0;

    uint64_t t = sctClock;

    // Return an error if not enough remaining events are available
    if ((s_currentEvent + 2) > FSL_FEATURE_SCT_NUMBER_OF_EVENTS)
    {
        return kStatus_Fail;
    }

    // Set unify bit to operate in 32-bit counter mode
    base->CONFIG |= SCT_CONFIG_UNIFY_MASK;

    // Calculate the ticks period with provide frequency [mHz]
	t *= 1000;
	t /= freq_mHz;
	period = t - 1;

    // Calculate the ticks pulse period with provided timing [us]
	t = pulseWidth_us;
	t*= sctClock;
	t /= 1000000U;
	pulsePeriod = t;
    //pulsePeriod = pulseWidth_us * (srcClock_Hz / 1000000U);

	// Check that the pulse period isn't bigger than the main period.
    if(pulsePeriod > period){
    	return kStatus_InvalidArgument;
    }

    // Schedule an event when the period is reached
    SCTIMER_CreateAndScheduleEvent(base, kSCTIMER_MatchEventOnly, period, 0, kSCTIMER_Counter_L, &periodEvent);

    // Schedule an event when the pulse width period is reached
    SCTIMER_CreateAndScheduleEvent(base, kSCTIMER_MatchEventOnly, pulsePeriod, 0, kSCTIMER_Counter_L, &pulseEvent);

    // Reset the counter when the period is reached
    SCTIMER_SetupCounterLimitAction(base, kSCTIMER_Counter_L, periodEvent);

    // Return the period event
    *event = periodEvent;

	// Set the initial output level to low which is the inactive state
	base->OUTPUT &= ~(1U << output);

	// Set the output when the period is reached.
	SCTIMER_SetupOutputSetAction(base, output, periodEvent);

	// Clear the output when the pulse period is reached
	SCTIMER_SetupOutputClearAction(base, output, pulseEvent);

    return kStatus_Success;
}

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
status_t IPULSE_UpdatePulseFrequency(SCT_Type *base, sctimer_out_t output, uint32_t srcClock_Hz, uint32_t freq_mHz, uint32_t event)

{
    assert(freq_mHz > 0);
    assert(srcClock_Hz > 0);
    assert(output < FSL_FEATURE_SCT_NUMBER_OF_OUTPUTS);

    uint32_t periodMatchReg, pulseMatchReg;
    uint32_t pulsePeriod, period = 0;
    uint32_t sctClock    = srcClock_Hz / (((base->CTRL & SCT_CTRL_PRE_L_MASK) >> SCT_CTRL_PRE_L_SHIFT) + 1);

    uint64_t t = sctClock;

    // Retrieve the match register number for the main period
    periodMatchReg = base->EVENT[event].CTRL & SCT_EVENT_CTRL_MATCHSEL_MASK;

    // Retrieve the match register number for the pulse period */
    pulseMatchReg = base->EVENT[event + 1].CTRL & SCT_EVENT_CTRL_MATCHSEL_MASK;

    pulsePeriod = base->SCTMATCH[pulseMatchReg];

    // Calculate the period
    t *= 1000;
    t /= freq_mHz;
    period = t;

    if(pulsePeriod > period ){
    	return kStatus_InvalidArgument;
    }

    // Stop the counter before updating match register
    SCTIMER_StopTimer(base, kSCTIMER_Counter_L);

    // Set the output level to low which is the inactive state
	base->OUTPUT &= ~(1U << output);

    // Update the main period
	base->SCTMATCH[periodMatchReg]    = period;
	base->SCTMATCHREL[periodMatchReg] = period;

    // Restart the counter
    SCTIMER_StartTimer(base, kSCTIMER_Counter_L);
    return kStatus_Success;
}

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
status_t IPULSE_UpdatePulseWidth(SCT_Type *base, sctimer_out_t output, uint32_t srcClock_Hz, uint32_t pulseWidth_us, uint32_t event)
{
    assert(pulseWidth_us > 0);
    assert(srcClock_Hz > 0);
    assert(output < FSL_FEATURE_SCT_NUMBER_OF_OUTPUTS);

    uint64_t t = pulseWidth_us;
    uint32_t periodMatchReg, pulseMatchReg;
    uint32_t pulsePeriod = 0, period;

    // Retrieve the match register number for the main period
    periodMatchReg = base->EVENT[event].CTRL & SCT_EVENT_CTRL_MATCHSEL_MASK;

    // Retrieve the match register number for the pulse period
    pulseMatchReg = base->EVENT[event + 1].CTRL & SCT_EVENT_CTRL_MATCHSEL_MASK;

    period = base->SCTMATCH[periodMatchReg];

    // Calculate pulse width match value
    t *= srcClock_Hz;
    t/= 1000000U;
    pulsePeriod = t;
   // pulsePeriod = (unsigned long)pulseWidth_us * (unsigned long)(srcClock_Hz / 1000000U);

    // If pulse period is greater than period, the event will never occur
    if (pulsePeriod >= period)
    {
    	return kStatus_InvalidArgument;
    }

    // Stop the counter before updating match register
    SCTIMER_StopTimer(base, kSCTIMER_Counter_L);

    // Set the output level to low which is the inactive state
	base->OUTPUT &= ~(1U << output);
    // Update pulse period
    base->SCTMATCH[pulseMatchReg]    = pulsePeriod;
    base->SCTMATCHREL[pulseMatchReg] = pulsePeriod;

    // Restart the counter
    SCTIMER_StartTimer(base, kSCTIMER_Counter_L);
    return kStatus_Success;
}


/**
 * @brief Set the enable state of the pulsing.
 * @param base              SCTimer peripheral base address.
 * @param output            The output to configure.
 * @param enable			State to set.
 * @return 'kStatus_Success'.
 */
status_t IPULSE_EnablePulse(SCT_Type *base, sctimer_out_t output, uint8_t enable){

	if(enable){

		// Start the counter
		SCTIMER_StartTimer(base, kSCTIMER_Counter_L);
	}
	else{
		// Stop the counter before updating match register
		SCTIMER_StopTimer(base, kSCTIMER_Counter_L);
		// Set output to low
		base->OUTPUT &= ~(1U << output);
	}
	return kStatus_Success;
}
