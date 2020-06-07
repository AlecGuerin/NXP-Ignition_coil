
/**
 * @file    LPC824_Ignition_Coil.c
 * @brief   Application entry point.
 * @author Alec Guerin
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "LPC824.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_sctimer.h"
#include "fsl_pint.h"
#include "fsl_syscon.h"

#include "ignition_pulse.h"
#include "SysTick.h"
#include "led.h"
#include "lcd.h"

#define SCTIMER_CLK_FREQ CLOCK_GetFreq(kCLOCK_Irc)	//! Get the clock frequency.


#define CMD_OUTPUT kSCTIMER_Out_0 ///< SCT timer output 0

//#define PINT_COD_CHA_INT0_SRC kSYSCON_GpioPort0Pin1ToPintsel	///<
#define PINT_COD_PUSH_INT1_SRC kSYSCON_GpioPort0Pin10ToPintsel	///<
#define PINT_SWITCH_INT2_SRC kSYSCON_GpioPort0Pin11ToPintsel	///<
#define PINT_COD_CHB_INT3_SRC kSYSCON_GpioPort0Pin15ToPintsel	///<

#define TR_TO_FREQ 33.333		///< Turn per minute multiplied by this value provide the frequency to set [mHZ].

#define DEFAULT_PULSE_WIDTH 2000	///< Default pulse width [ms].

#define DEFAULT_TR_MIN 6900
#define MIN_TR_MIN 2500
#define MAX_TR_MIN 9000

#define DEFAULT_FREQ_mHZ 226667
#define MIN_FREQ_mHZ  70000
#define MAX_FREQ_mHZ  300000

#define TR_MIN_INQ  10		///< RPM increment.

#define STATE_STACK_COUNT 8	///< State stack size

#define SCREEN_RMP_OFFSET 48
#define SCREEN_MATCH_OFFSET 0
#define SCREEN_RUN_OFFSET 76

enum STATE {
	NONE = 0,         //!< NONE
	ADD_TR,           //!< ADD_TR
	REM_TR,           //!< REM_TR
	ENABLE_PULSES,    //!< ENABLE_PULSES
	UPDATE_PULSES,    //!< UPDATE_PULSES
	UPDATE_PULSE_WIDTH//!< CHANGE_PULSE_WIDTH
};

enum SCREEN_LINE{
	MSG_S_RPM =1,
	S_RPM,
	MSG_R_RPM,
	R_RPM,
	STATUS = 6
};

volatile uint32_t _switchOn = 0;

volatile uint32_t _intA = 0;

volatile uint8_t _stateCounter = 0;
volatile uint8_t _state = NONE;

volatile uint32_t _event;
volatile uint32_t _sctimerClock;

volatile uint32_t _nextAllowedInter = 0;
volatile uint32_t _rpmInq = 10;

volatile uint32_t _stateStack[STATE_STACK_COUNT];

uint32_t trToHz(uint32_t trMin);
uint32_t GetTickCount(void);
void UpdateNextAllowedInter(void);

void PushState(uint32_t state);
void UpdateState(void);

//void CoderA_callback(void);
void CoderB_callback(void);
void CoderPush_callback(void);
void Switch_callback(void);

/**
 * Initialize the board and chip.
 */
int init(void) {

	int res = 1;

	PINT_Init(PINT);
	BOARD_InitPins();
	BOARD_BootClockIRC12M();
	BOARD_InitSWD_DEBUGPins();

	SYSTICK_Init_ms(1);

	LED_SetLed(LED_RED_LED, 1);
	LCD_Init();
	LED_SetLed(LED_RED_LED, 0);

	/* Enable clock of sct. */
	CLOCK_EnableClock(kCLOCK_Sct);

	LED_SetLed(LED_GREEN_LED, 1);

	// Set interruptions
	//SYSCON_AttachSignal(SYSCON, kPINT_PinInt0, PINT_COD_CHA_INT0_SRC);
	SYSCON_AttachSignal(SYSCON, kPINT_PinInt1, PINT_COD_PUSH_INT1_SRC);
	SYSCON_AttachSignal(SYSCON, kPINT_PinInt2, PINT_SWITCH_INT2_SRC);
	SYSCON_AttachSignal(SYSCON, kPINT_PinInt3, PINT_COD_CHB_INT3_SRC);

	LED_SetLed(LED_RED_LED, 1);

	LED_SetLed(LED_GREEN_LED, 0);

    /* Connect trigger sources to PINT */
	//PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableFallEdge, CoderA_callback);
	PINT_PinInterruptConfig(PINT, kPINT_PinInt1, kPINT_PinIntEnableRiseEdge,CoderPush_callback);
	PINT_PinInterruptConfig(PINT, kPINT_PinInt2, kPINT_PinIntEnableBothEdges, Switch_callback);
	PINT_PinInterruptConfig(PINT, kPINT_PinInt3, kPINT_PinIntEnableFallEdge, CoderB_callback);

	//PINT_EnableCallbackByIndex(PINT, kPINT_PinInt0);
	PINT_EnableCallbackByIndex(PINT, kPINT_PinInt1);
	PINT_EnableCallbackByIndex(PINT, kPINT_PinInt2);
	PINT_EnableCallbackByIndex(PINT, kPINT_PinInt3);

	return res;
}

/*
 * @brief   Application entry point.
 */
int main(void) {

	//uint8_t temp = 0;						// Temporary variable to use if needed
	uint8_t pwmEnable = 0;					// Does the PWM is enabled.

	uint32_t cmdRpm = DEFAULT_TR_MIN;		// Command RPM.
	uint32_t currentRpm = DEFAULT_TR_MIN;	// Current (running) RPM.
	uint32_t pulseWidth = DEFAULT_PULSE_WIDTH;	// Pulse width [us].

	sctimer_config_t sctimerInfo;		// SC timer information.
	char sRpm[10];						// Characters array used to display integers.


	init();		// Initialize the board and fixtures.

	LED_SetAll();

	_stateStack[0] = _state;
	_sctimerClock = SCTIMER_CLK_FREQ;

	SCTIMER_GetDefaultConfig(&sctimerInfo);
    SCTIMER_Init(SCT0, &sctimerInfo);

    if (IPULSE_SetupPulse(SCT0, DEFAULT_PULSE_WIDTH, _sctimerClock, trToHz(cmdRpm), CMD_OUTPUT, &_event) !=
    		kStatus_Success)
	{
		return -1;
	}


    LCD_DisplayClear(0x00,0x00);

    sprintf(sRpm, "%d", cmdRpm);
    LCD_DisplayString(MSG_S_RPM,4,"Set RPM:");
    LCD_DisplayString(S_RPM,SCREEN_RMP_OFFSET,sRpm);

    LCD_DisplayString(MSG_R_RPM,4,"Running RPM:");
	LCD_DisplayString(R_RPM,SCREEN_RMP_OFFSET,sRpm);

	LCD_DisplayString(STATUS,SCREEN_MATCH_OFFSET,"RMP match: YES");
    LCD_DisplayString(STATUS,SCREEN_RUN_OFFSET,"State: OFF");


    LED_ResetAll();

    /* Enter an infinite loop, just incrementing a counter. */
    while(1) {

    	switch(_state){

    	case NONE:
    		__asm volatile ("nop");
    		break;

    	case ADD_TR:

			if(cmdRpm >= MAX_TR_MIN - _rpmInq){
				cmdRpm = MAX_TR_MIN;
			}
			else{
				cmdRpm += _rpmInq;
			}

			sprintf(sRpm, "%d", cmdRpm);
			LCD_DisplayString(S_RPM,SCREEN_RMP_OFFSET,sRpm);

			if(cmdRpm != currentRpm){
				LED_SetLed(LED_RED_LED, 1);
				LCD_DisplayRectangle(56, STATUS, 16, 1 , 0x00);
				LCD_DisplayString(STATUS,SCREEN_MATCH_OFFSET,"RMP match: NO");
			}
			else{
				LED_SetLed(LED_RED_LED, 0);
				LCD_DisplayString(STATUS,SCREEN_MATCH_OFFSET,"RMP match: YES");
			}

			UpdateState();
    		break;

    	case REM_TR:

			if(cmdRpm <= MIN_TR_MIN + _rpmInq){
				cmdRpm = MIN_TR_MIN;
			}
			else{
				cmdRpm -= _rpmInq;
			}

			sprintf(sRpm, "%d", cmdRpm);
			LCD_DisplayString(S_RPM,SCREEN_RMP_OFFSET,sRpm);

			if(cmdRpm != currentRpm){
				LED_SetLed(LED_RED_LED, 1);
				LCD_DisplayRectangle(56, STATUS, 16, 1 , 0x00);
				LCD_DisplayString(STATUS,SCREEN_MATCH_OFFSET,"RMP match: NO");
			}
			else{
				LED_SetLed(LED_RED_LED, 0);
				LCD_DisplayString(STATUS,SCREEN_MATCH_OFFSET,"RMP match: YES");
			}

			UpdateState();
    		break;

    	case ENABLE_PULSES:

			LED_ResetAll();

			pwmEnable = !pwmEnable;

			IPULSE_EnablePulse(SCT0, CMD_OUTPUT, pwmEnable);
			LED_SetLed(LED_GREEN_LED, pwmEnable);
			UpdateState();

			if(pwmEnable){
				LCD_DisplayRectangle(117, STATUS, 10, 1 , 0x00);
				LCD_DisplayString(STATUS,SCREEN_RUN_OFFSET,"State: ON");
			}
			else{
				LCD_DisplayString(STATUS,SCREEN_RUN_OFFSET,"State: OFF");
			}
			PushState(UPDATE_PULSES);

    		break;

    	case UPDATE_PULSES:

    		if(cmdRpm > MAX_TR_MIN){
				cmdRpm = MAX_TR_MIN;
			}else if(cmdRpm < MIN_TR_MIN){
				cmdRpm = MIN_TR_MIN;
			}

			IPULSE_UpdatePulseFrequency(SCT0,CMD_OUTPUT, _sctimerClock, trToHz(cmdRpm), _event);
			IPULSE_EnablePulse(SCT0, CMD_OUTPUT, pwmEnable);

			currentRpm = cmdRpm;

			sprintf(sRpm, "%d", currentRpm);
			LCD_DisplayString(R_RPM,SCREEN_RMP_OFFSET,sRpm);
			LCD_DisplayString(STATUS,SCREEN_MATCH_OFFSET,"RMP match: YES");
			LED_SetLed(LED_RED_LED, 0);

			UpdateState();
    		break;

    	case UPDATE_PULSE_WIDTH:

			IPULSE_UpdatePulseWidth(SCT0,CMD_OUTPUT, _sctimerClock, pulseWidth, _event);
			UpdateState();
    		break;

    	default:

    		break;

    	}
    }
    return 0 ;
}

/**
 *
 * @param trMin
 * @return
 */
uint32_t trToHz(uint32_t trMin){

	uint32_t freq = DEFAULT_FREQ_mHZ;

	// Check max values
	if(trMin > MAX_TR_MIN){
		trMin = MAX_TR_MIN;
	}
	else if(trMin < MIN_TR_MIN){
		trMin = MIN_TR_MIN;
	}

	freq = trMin * TR_TO_FREQ;

	return freq;
}

/**
 *
 */
void UpdateState(void){
	if(_stateCounter)
		_stateCounter --;

	_state = _stateStack[_stateCounter];

}

void PushState(uint32_t state){

	if(_stateCounter < STATE_STACK_COUNT){
		_stateCounter ++;
		_stateStack[_stateCounter] = state;
		_state = _stateStack[_stateCounter];
	}
}

/**
 *
 */
/*void CoderA_callback(void){

}*/

/**
 **
 */
void CoderB_callback(void){

	_intA = GPIO_PinRead(GPIO, 0, 1);

	if( SYSTICK_GetTicks() > _nextAllowedInter){

		if(_intA){
			PushState(REM_TR);
		}
		else{
			PushState(ADD_TR);
		}
		UpdateNextAllowedInter();
	}

}

/**
 *
 */
void CoderPush_callback(void){

	if(SYSTICK_GetTicks() > _nextAllowedInter){

		PushState(ENABLE_PULSES);
		UpdateNextAllowedInter();
	}
}

/**
 *
 */
void Switch_callback(void){
	if(SYSTICK_GetTicks() > _nextAllowedInter){
		_switchOn = !_switchOn;

		if(_switchOn){
			_rpmInq = 1;
		}
		else{
			_rpmInq = 10;
			PushState(UPDATE_PULSES);
		}
		UpdateNextAllowedInter();
	}
}

void UpdateNextAllowedInter(void){
	_nextAllowedInter =  SYSTICK_GetTicks()+ 15;
}
