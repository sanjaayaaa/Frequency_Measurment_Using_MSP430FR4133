#include "LCD.h"
#include "string.h"
#include "driverlib.h"
#include "main.h"

volatile unsigned char * mode = &BAKMEM4_L;
volatile unsigned int * unit = &BAKMEM0_L;
volatile unsigned int * ten = &BAKMEM0_H;
volatile unsigned int * hun = &BAKMEM1_L;
volatile unsigned int * thou = &BAKMEM1_H;
volatile unsigned int * tenThou = &BAKMEM1_L;

const char digit[10][2] =
{
    {0xFC, 0x28},  /* "0" LCD segments a+b+c+d+e+f+k+q */
    {0x60, 0x20},  /* "1" */
    {0xDB, 0x00},  /* "2" */
    {0xF3, 0x00},  /* "3" */
    {0x67, 0x00},  /* "4" */
    {0xB7, 0x00},  /* "5" */
    {0xBF, 0x00},  /* "6" */
    {0xE4, 0x00},  /* "7" */
    {0xFF, 0x00},  /* "8" */
    {0xF7, 0x00}   /* "9" */
};

const char alphabetBig[26][2] =
{
    {0xEF, 0x00},  /* "A" LCD segments a+b+c+e+f+g+m */
    {0xF1, 0x50},  /* "B" */
    {0x9C, 0x00},  /* "C" */
    {0xF0, 0x50},  /* "D" */
    {0x9F, 0x00},  /* "E" */
    {0x8F, 0x00},  /* "F" */
    {0xBD, 0x00},  /* "G" */
    {0x6F, 0x00},  /* "H" */
    {0x90, 0x50},  /* "I" */
    {0x78, 0x00},  /* "J" */
    {0x0E, 0x22},  /* "K" */
    {0x1C, 0x00},  /* "L" */
    {0x6C, 0xA0},  /* "M" */
    {0x6C, 0x82},  /* "N" */
    {0xFC, 0x00},  /* "O" */
    {0xCF, 0x00},  /* "P" */
    {0xFC, 0x02},  /* "Q" */
    {0xCF, 0x02},  /* "R" */
    {0xB7, 0x00},  /* "S" */
    {0x80, 0x50},  /* "T" */
    {0x7C, 0x00},  /* "U" */
    {0x0C, 0x28},  /* "V" */
    {0x6C, 0x0A},  /* "W" */
    {0x00, 0xAA},  /* "X" */
    {0x00, 0xB0},  /* "Y" */
    {0x90, 0x28}   /* "Z" */
};

void Inti_GPIO()
{
    //Port select XT1
        GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,GPIO_PIN1 + GPIO_PIN2,GPIO_PRIMARY_MODULE_FUNCTION);
}

void Init_Clock()
{
    //Set external frequency for XT1
	CS_setExternalClockSource(32768);

	//Select XT1 as the clock source for ACLK with no frequency divider
	CS_initClockSignal(CS_ACLK, CS_XT1CLK_SELECT, CS_CLOCK_DIVIDER_1);

    //Start XT1 with no time out
    CS_turnOnXT1(CS_XT1_DRIVE_0);

    //clear all OSC fault flag
    CS_clearAllOscFlagsWithTimeout(1000);
}


void Init_LCD()
{
    // L0~L26 & L36~L39 pins selected
    LCD_E_setPinAsLCDFunctionEx(LCD_E_BASE, LCD_E_SEGMENT_LINE_0, LCD_E_SEGMENT_LINE_26);
    LCD_E_setPinAsLCDFunctionEx(LCD_E_BASE, LCD_E_SEGMENT_LINE_36, LCD_E_SEGMENT_LINE_39);

    LCD_E_initParam initParams = LCD_E_INIT_PARAM;
    initParams.clockDivider = LCD_E_CLOCKDIVIDER_3;
    initParams.muxRate = LCD_E_4_MUX;
    initParams.segments = LCD_E_SEGMENTS_ENABLED;

    // Init LCD as 4-mux mode
    LCD_E_init(LCD_E_BASE, &initParams);

    LCD_E_setVLCDSource(LCD_E_BASE, LCD_E_INTERNAL_REFERENCE_VOLTAGE, LCD_E_EXTERNAL_SUPPLY_VOLTAGE);
    LCD_E_setVLCDVoltage(LCD_E_BASE, LCD_E_REFERENCE_VOLTAGE_2_96V);

    LCD_E_enableChargePump(LCD_E_BASE);
    LCD_E_setChargePumpFreq(LCD_E_BASE, LCD_E_CHARGEPUMP_FREQ_16);

    // Clear LCD memory
    LCD_E_clearAllMemory(LCD_E_BASE);

    // Configure COMs and SEGs
    // L0 = COM0, L1 = COM1, L2 = COM2, L3 = COM3
    LCD_E_setPinAsCOM(LCD_E_BASE, LCD_E_SEGMENT_LINE_0, LCD_E_MEMORY_COM0);
    LCD_E_setPinAsCOM(LCD_E_BASE, LCD_E_SEGMENT_LINE_1, LCD_E_MEMORY_COM1);
    LCD_E_setPinAsCOM(LCD_E_BASE, LCD_E_SEGMENT_LINE_2, LCD_E_MEMORY_COM2);
    LCD_E_setPinAsCOM(LCD_E_BASE, LCD_E_SEGMENT_LINE_3, LCD_E_MEMORY_COM3);

    // Select to display main LCD memory
    LCD_E_selectDisplayMemory(LCD_E_BASE, LCD_E_DISPLAYSOURCE_MEMORY);

    // Turn on LCD
    LCD_E_on(LCD_E_BASE);
}


void displayScrollText(char *msg)
{
    int length = strlen(msg);
    int oldmode = *mode;
    int i;
    int s = 5;
    char buffer[6] = "      ";
    for (i=0; i<length+7; i++){
		if (*mode != oldmode)
            break;
        int t;
        for (t=0; t<6; t++)
            buffer[t] = ' ';
        int j;
        for (j=0; j<length; j++){
            if (((s+j) >= 0) && ((s+j) < 6))
                buffer[s+j] = msg[j];
        }
        s--;
        showChar(buffer[0], pos1);
        showChar(buffer[1], pos2);
        showChar(buffer[2], pos3);
        showChar(buffer[3], pos4);
        showChar(buffer[4], pos5);
        showChar(buffer[5], pos6);
        __delay_cycles(200000);
}
}

void showChar(char c, int position)
{
    if (c == ' '){
		LCDMEMW[position/2] = 0;
    } else if (c >= '0' && c <= '9'){
		LCDMEMW[position/2] = digit[c-48][0] | (digit[c-48][1] << 8);
    } else if (c >= 'A' && c <= 'Z'){
		LCDMEMW[position/2] = alphabetBig[c-65][0] | (alphabetBig[c-65][1] << 8);
    } else {
		LCDMEMW[position/2] = 0xFFFF;
    }
}

void displayFrequency(unsigned int frequency)
{
	unsigned int frequencyDivByTen,frequencyDivByHundred,frequencyDivByThousand,frequencyDivByTenThousand;
	
	// Logic To Send Frequency serialy on LCD
	unit = &frequency;
	frequencyDivByTen=*unit/10;
	ten=&frequencyDivByTen;
	frequencyDivByHundred=*ten/10;
	hun=&frequencyDivByHundred;
	frequencyDivByThousand=*hun/10;
	thou =&frequencyDivByThousand;
	frequencyDivByTenThousand=*thou/10;
	tenThou=&frequencyDivByTenThousand;

	showChar('H' ,sixPosition);
	showChar((*unit) % 10+'0',fifthPosition);
	showChar((*ten) % 10 +'0' ,fourthPosition);
	showChar((*hun) % 10 +'0' ,thirdPosition);
	showChar((*thou) % 10 +'0' ,secondPosition);
	showChar((*tenThou) % 10 +'0' ,firstPosition);
}

void clearLCD()
{
    LCDMEMW[firstPosition/2] = 0;
    LCDMEMW[secondPosition/2] = 0;
    LCDMEMW[thirdPosition/2] = 0;
    LCDMEMW[fourthPosition/2] = 0;
    LCDMEMW[fifthPosition/2] = 0;
    LCDMEMW[sixPosition/2] = 0;
    LCDMEM[12] = LCDMEM[13] = 0;
}
