#include "LCD.h"
#include "string.h"
#include "driverlib.h"
#include "main.h"
#include "msp430fr4133.h"

volatile unsigned int captures = 0;
volatile unsigned int startTime = 0;
volatile unsigned int frequencyInHz = 0;
volatile unsigned int frequencyInKHZ = 0;

void main(void)
{
    WDT_A_hold(WDT_A_BASE);

	// TA0.CCI2A input capture pin, second function
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    // Set as ACLK pin, second function
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P8, GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);
    PMM_unlockLPM5();
    Inti_GPIO();
    Init_Clock();
    Init_LCD();
    displayScrollText("FREQUENCY MEASURMENT");
    Timer_init();
	while (1){
		__bis_SR_register(LPM1_bits + GIE);
        __no_operation();
		}
}

void Timer_init()
{
	Timer_A_initContinuousModeParam param = {0};
	param.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
	param.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
	param.timerClear = TIMER_A_DO_CLEAR;
	param.startTimer = false;
	Timer_A_initContinuousMode(TIMER_A0_BASE, &param);

   // Capture on CCI2A,Rising edge , no prescale, sync
	Timer_A_initCaptureModeParam param1 = {0};
	param1.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2 ;
	param1.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE ;
	param1.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
	param1.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
	param1.captureInterruptEnable =TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
	param1.captureOutputMode =TIMER_A_OUTPUTMODE_OUTBITVALUE;
	Timer_A_initCaptureMode(TIMER_A0_BASE, &param1);
	Timer_A_clearTimerInterrupt(TIMER_A0_BASE);
	Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_2);
	Timer_A_startCounter( TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void)
{
    switch(__even_in_range( TA0IV, 6 )){
		case 0x00: break;
		case 0x02:break;
		case 0x04:
			switch (captures){
				case 0:{
					startTime = Timer_A_getCaptureCompareCount(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_2);
					captures =  1;
					GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
					break ;
				}
				case 1:{
					frequencyInHz = ((Timer_A_getCaptureCompareCount(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_2)-startTime)/4);
					frequencyInKHZ = (1000000/frequencyInHz);
					displayFrequency (frequencyInKHZ);
					captures = 0;
					Timer_A_disableCaptureCompareInterrupt (TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_2);
					GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
					break;
				}
				default :{
				  -- captures;
				  break;
				}
				break;
			}
		case 0x06:
			break;
		default: 
			_never_executed();
      }
}
