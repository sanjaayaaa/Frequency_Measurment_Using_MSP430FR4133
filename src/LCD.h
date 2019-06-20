#include <msp430fr4133.h>

#ifndef LCD_H_
#define LCD_H_


#define LCDMEMW ((int*)LCDMEM)

#ifdef __IAR_SYSTEMS_ICC__
#define LCDBMEMW ((int*)&LCDM32)
#else
#define LCDBMEMW ((int*)LCDBMEM)
#endif

#define firstPosition 4   	/* Digit A1 - L4  */
#define secondPosition 6   	/* Digit A2 - L6  */
#define thirdPosition 8   	/* Digit A3 - L8  */
#define fourthPosition 10  	/* Digit A4 - L10 */
#define fifthPosition 2   	/* Digit A5 - L2  */
#define sixPosition 18  		/* Digit A6 - L18 */

void Inti_GPIO();
void Init_Clock();
void Init_LCD();
void clearLCD();
void displayScrollText(char *msg);
void showChar(char c, int position);
void displayFrequency(unsigned int frequency);
unsigned int frequency;

extern const char digit[10][2];
extern const char alphabetBig[26][2];

#endif
