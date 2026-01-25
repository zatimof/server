
#define NCYCLE 20 					//количество циклов на длинное нажатие

#define ADC_CH 5,6,7			//сканируемые каналы АЦП
#define NUM_OF_ADC_CH 3
#define BAT_COEF 146
#define OUT_COEF 150
#define LED_COEF 208

#define DISP_TIME_ON 250		//длительность включенного дисплея

#define BEEP_ON PORTC&=0xF7
#define BEEP_OFF PORTC|=0x08

#define BUTTON PINC & 0x02		//TEST BUTTON

#define BAT_OFF_LEVEL 75
#define BAT_LOW_LEVEL 100
#define BAT_CH12_LEVEL 112
#define BAT_CH23_LEVEL 118
#define BAT_NOM_LEVEL 124
#define BAT_MAX_LEVEL 126

#include <iom328p.h>
#include <intrinsics.h> 

//functions declare;
void initialize(void);																//initialize function
void display(void);
void fill_disp(void);
void read_keyb(void);
void logic(void);
void send(unsigned char data,unsigned char dc);				//SPI send function