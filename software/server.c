//Управляющая программа микроконтроллера ATmega328P блока бесперебойного питания v.1.0
//Управление зарядом (HIGH - ACTIVE)
//PB0 - выключить повышающий преобразователь
//PB1 - включить резистор 1.65 Ом
//PB2 - включить резистор 1.1 Ом
//Распиновка индикатора SPI:
//PD3 - D0 - SCK
//PD4 - D1 - SDIN
//PD5 - RES - RESET (LOW - ACTIVE)
//PD6 - DC - DATA/COMMAND (LOW - COMMAND, HIGH - DATA)
//PD7 - CS - CHIP SELECT (LOW - ACTIVE)
//Распиновка АЦП
//PC5 - вход состояния светодиодов
//ADC6 - Ubat
//ADC7 - Uout
//Остальные функции:
//PD0 - вход датчика переменного напряжения
//PD1 - вход контроля повышающего преобразователя
//PD2 - вход 5В (не используется)
//PC0 - выход управления вентилятором
//PC1 - кнопка TEST
//PC2 - NC
//PC3 - бипер (LOW - ACTIVE)
//PC4 - выход управления кнопкой включения

//Автор Захаров Т. Р. 2025 г.

#include "server.h"

//global variables declare
//массив индикатора 8 строк по 16 знакомест
unsigned char screen[8][16]={	{11,10,28,28,14,26,32,16,29,18,16,16,16,16,16,30},
															{ 0,29,28,25,29,28,16,16,29,18,16,16,16,16,16,30},
															{12,21,10,26,20,14,16,16,22,18,16,16,16,16,16,10},
															{17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17},
															{27,14,26,30,14,26,16,16,16,16,16,16,16,16,16,16},
															{11,10,28,28,14,26,32,16,16,16,16,16,16,16,16,16},
															{17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17},
															{16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16}};			
//universal variables
unsigned char i, disp_on = DISP_TIME_ON,  blink = 0, btn = 0;

//AC control variables
unsigned char ac_ok = 0, dc_ok = 0, ac_cnt = 0, ac = 0;

//Server control variables
unsigned char last_code[2], cur_code = 0, dur[2], press = 0, code_cnt = 0, code = 0, user_off = 0;

//Battery control variables
unsigned char charge = 0, u_bat, u_out, i_ch, test = 0;

//adc variables
unsigned char sel_adc[NUM_OF_ADC_CH]={ADC_CH}, cur_adc=0;
int adc[NUM_OF_ADC_CH];

//time count variables
unsigned char time = 0;
unsigned int active_cnt = 0;

//enum
enum {OFF_S, IDLE, START, ACTIVE_S, STOP} serv_state = OFF_S;
enum {OFF_B, TEST, CHARGE, ACTIVE_B, DISCH, BAD} batt_state = OFF_B;

//digit codegen
__flash const char img[33][5]={
{0x3E,0x41,0x41,0x41,0x3E},			//0										
{0x44,0x42,0x7F,0x40,0x40},			//1
{0x42,0x61,0x51,0x49,0x46},			//2
{0x21,0x41,0x45,0x4B,0x31},			//3
{0x18,0x14,0x12,0x7F,0x10},			//4
{0x27,0x45,0x45,0x45,0x39},			//5
{0x3C,0x4A,0x49,0x49,0x30},			//6
{0x01,0x71,0x09,0x05,0x03},			//7
{0x36,0x49,0x49,0x49,0x36},			//8
{0x06,0x49,0x49,0x29,0x1E},			//9
{0x7E,0x11,0x11,0x11,0x7E},		//A	10
{0x7F,0x49,0x49,0x49,0x36},		//B	11
{0x3E,0x41,0x41,0x41,0x22},		//C	12
{0x7F,0x41,0x41,0x41,0x3E},		//D	13
{0x7F,0x49,0x49,0x49,0x49},		//E	14
{0x7F,0x09,0x09,0x09,0x09},		//F	15
{0x00,0x00,0x00,0x00,0x00},		//_	16 (пробел)
{0x08,0x08,0x08,0x08,0x08},		//-	17
{0x00,0x00,0x36,0x36,0x00},		//:	18
{0x00,0x00,0x60,0x60,0x00},		//.	19
{0x3E,0x41,0x41,0x51,0x32},		//G 20
{0x7F,0x08,0x08,0x08,0x7F},		//H 21
{0x00,0x41,0x7F,0x41,0x00},		//I	22
{0x7F,0x40,0x40,0x40,0x40},		//L 23
{0x7F,0x06,0x18,0x06,0x7F},		//M 24
{0x7F,0x09,0x09,0x09,0x06},		//P	25
{0x7F,0x09,0x09,0x19,0x66},		//R	26
{0x26,0x49,0x49,0x49,0x32},		//S	27
{0x01,0x01,0x7F,0x01,0x01},		//T	28
{0x3F,0x40,0x40,0x40,0x3F},		//U	29
{0x07,0x38,0x40,0x38,0x07},		//V 30
{0x3F,0x40,0x3E,0x40,0x3F},		//W 31
{0x03,0x0C,0x70,0x0C,0x03}};	//Y 32

//OLED init string 1
__flash const char init1[23]={0x75,0xAB,0x09,0x15,
0xFC,0xCB,0x00,0x02,0xB1,0x28,0x85,0x13,0x5B,0x48,
0x81,0x0D,0x9B,0x44,0xDB,0x0C,0x25,0x65,0xF5};				

//OLED init string 2
__flash const char init2[8]={0x04,0x00,0x84,0x00,0xFE,0x44,0x00,0xE0};

//main function
void main(void)
{
  initialize();										//начальная инициализация
	
  while(1)
  {
    PORTB^=0x20;									//led blink
		blink++;
		
		logic();											//main logic
		
		//disp_on=10;										//display off disable
		if(disp_on)										//display on timeout
			disp_on--;
		
		fill_disp();
		display();										//screen refresh
		
		read_keyb();									//keyb read
		
		__watchdog_reset();						//watchdog reset
		
    __delay_cycles(1000000);			//delay
  }
}

//initialize function
void initialize(void)
{
//init ports  
	//DDRB = 0x27;
	DDRB = 0x07;
	PORTB = 0x01;
	
	DDRD = 0xF8;
	PORTD = 0x00;
	
	DDRC = 0x09;
	PORTC = 0x0A;
	
	MCUCR = 0x00;

//init timer indicator
	TCCR0A = 0x00;
	TCCR0B = 0x03;
	TIMSK0 = 0x01;
	GTCCR = 0x00;
	
	TCCR1A = 0x00;
	TCCR1B = 0x03;
	TCCR1C = 0x00;
	TIMSK1 = 0x01;
	
	TCCR2A = 0x00;
	TCCR2B = 0x07;
	TIMSK2 = 0x01;
	
//init ADC
	ADMUX = 0x40 + sel_adc[0];
	ADCSRA = 0xCF;
	ADCSRB = 0x00;
	
//init WatchDog
	__watchdog_reset();
	WDTCSR |= 0x18;
	WDTCSR = 0x1F;
	
//init interrupts
	SREG |= 128;

//init display
	BEEP_ON;
	PORTD |= 0x20;									//снимем сигнал сброса
	
	for(i=0;i<23;i++)								//загрузка строки инициализации
		send(init1[i],0);
	
	__delay_cycles(1000000);
	
	for(i=0;i<8;i++)								//загрузка второй строки инициализации
		send(init2[i],0);

	BEEP_OFF;
	
	return;
}

#pragma vector=ADC_vect						//прерывание по АЦП
__interrupt void adcf(void)
{
	int adct;	

	ADCSRA = 0x0F;
	adct = ADC;
	adc[cur_adc] += (adct - adc[cur_adc]) >> 1;
	//adc[cur_adc]=adct;
	cur_adc ++;
	if(cur_adc == NUM_OF_ADC_CH)
		cur_adc = 0;
	ADMUX &= 0xF0;
	ADMUX |= sel_adc[cur_adc];
	ADCSRA = 0xCF;
	
	return;
}

#pragma vector=TIMER0_OVF_vect						//прерывание по таймеру
__interrupt void timer0(void)
{
	if(PIND & 0x01)
		ac = 0;
	else
		if(!ac)
		{
			ac = 1;

			if((TCNT1 > 2500) && (TCNT1 < 7500) && (!ac_ok))
				ac_cnt++;
			else
				ac_cnt = 0;
			
			if(ac_cnt > 10)
				ac_ok = 1;
				
			TCNT1 = 0;
		}
	
	if(PIND & 0x02)
		dc_ok = 1;
	else
		dc_ok = 0;
	
	return;
}

#pragma vector=TIMER1_OVF_vect						//прерывание по таймеру
__interrupt void timer1(void)
{
	ac_ok = 0;	
	return;
}

void read_keyb(void)
{
	if(BUTTON)
		btn = 0;
	else
		if((btn == 0) || (btn == 1))
		{
			if((btn == 0) || (!test))
			{
				BEEP_ON;										//beep on
				__delay_cycles(100000);
				BEEP_OFF;										//beep off
			}
						
			if((btn == 0) && (!test))
				btn = NCYCLE;
			
			if((btn == 0) && (test))
				test = 2;
			
			if((btn == 1) && (!test))
				test = 1;
			
			disp_on = DISP_TIME_ON;
		}
		else
			btn--;

	return;
}

#pragma vector=TIMER2_OVF_vect						//прерывание по таймеру
__interrupt void timer2(void)
{
	if(press)
	{
		press--;
		
		if(press == 80)
		{
			DDRC = 0x19;
			BEEP_ON;
		}
		
		if(press == 60)
		{
			DDRC = 0x09;
			BEEP_OFF;
		}
		
		if(press == 20)
		{
			DDRC = 0x19;
			BEEP_ON;
		}
		
		if(press == 0)
		{			
			DDRC = 0x09;
			BEEP_OFF;
		}
	}
	
	if(time != 0xFF)
		time++;
	if(code_cnt != 0xFF)
		code_cnt++;
	if(active_cnt != 0xFFFF)
		active_cnt++;
	
	unsigned char t = ((long)adc[0] * LED_COEF) >> 14;

	if(t != code)
	{
		code = t;
		code_cnt = 0;
	}
	
	if((code != last_code[cur_code]) && (code_cnt > 5))
	{
		cur_code++;
		cur_code &= 0x01;
		last_code[cur_code] = code;
		
		if(time < 10)
			dur[cur_code] = 0;
		if((time > 10) && (time < 25))
			dur[cur_code] = 1;
		if((time > 25) && (time < 40))
			dur[cur_code] = 2;
		if(time > 40)
			dur[cur_code] = 3;
		
		time = 0;
	}
	
	return;
}