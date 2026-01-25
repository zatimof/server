#include "..\server.h"

extern unsigned char screen[8][16], disp_on, test, charge, ac_ok, dc_ok;
extern unsigned char u_bat, u_out, i_ch, last_code[], cur_code, dur[];
__flash const extern char img[33][5], init2[8];
extern int adc[NUM_OF_ADC_CH];
extern enum {OFF_S, IDLE, START, ACTIVE_S, STOP} serv_state;
extern enum {OFF_B, TEST, CHARGE, ACTIVE_B, DISCH, BAD} batt_state;
extern unsigned int active_cnt;

//Функция обновления индикатора. Передает образ дисплея screen[][] в индикатор.
void display(void)
{
	unsigned char i, k, line;
	
	for(i = 0; i < 8; i++)																//установка указателей на 
		send(init2[i], 0);															//начало экрана
	
	for(line = 7; line != 255; line--)												//цикл по строкам
		for(i = 127; i != 255; i--)														//цикл по столбцам
		{
			k = i & 0x07;

			if((k > 1) && (k < 7) && disp_on)										//вывод строки сообщения
				send(img[screen[line][i >> 3]][k - 2] << 1, 1);
			else
				send(0, 1);
		}

	return;
}

//Функцяи пересылки байта в индикатор по SPI.
void send(unsigned char data,unsigned char dc)
{
  if(dc)																					//выставляем сигнал DC и CS
    PORTD |= 0x40;
  else
    PORTD &= 0xBF;
	
	PORTD &= 0x7F;																		//выставляем сигнал CS
  
  for(unsigned char l = 0; l < 8; l++)								//цикл из 8 бит
  {
    PORTD &= 0xF7;																	//формируем спад CLK
		
    if(data & (0x01 << l))														//выставляем данные
      PORTD |= 0x10;
		else
			PORTD &= 0xEF;
		
		PORTD |= 0x08;																	//формируем фронт CLK
  }
  
  PORTD |= 0x80;																		//убираем сигнал CS
	
	return;
}

//Функция заполнения образа дисплея данными.
void fill_disp(void)
{
	unsigned char i;
	long t;

//расчет и вывод напряжений
	
	t = ((long)adc[1] * BAT_COEF) >> 10;
	u_bat = t;
		
	if(t > 145)
	{
		screen[0][11] = 0;
		screen[0][12] = 30;
		screen[0][13] = 26;
		screen[0][14] = 16;	
	}
	else
		if(t < 70)
		{
			screen[0][11] = 23;
			screen[0][12] = 0;
			screen[0][13] = 31;
			screen[0][14] = 16;	
		}
		else
		{
			i = 0;																				//преобразование в двоично-
			while(t >= 100)																//десятичный вид и вывод
			{
				t -= 100;
				i++;
			}
			screen[0][11] = i;
			screen[0][13] = 19;
			
			i = 0;
			while(t >= 10)																//десятичный вид и вывод
			{
				t -= 10;
				i ++;
			}
			screen[0][12] = i;
			screen[0][14] = t;
		}
	
	t = ((long)adc[2] * OUT_COEF) >> 10;
	u_out = t;
	
	if(t > 145)
	{
		screen[1][11] = 0;
		screen[1][12] = 30;
		screen[1][13] = 26;
		screen[1][14] = 16;	
	}
	else
		if(t < 70)
		{
			screen[1][11] = 23;
			screen[1][12] = 0;
			screen[1][13] = 31;
			screen[1][14] = 16;	
		}
		else
		{
			i = 0;																				//преобразование в двоично-
			while(t >= 100)																//десятичный вид и вывод
			{
				t -= 100;
				i++;
			}
			screen[1][11] = i;
			screen[1][13] = 19;
			
			i = 0;
			while(t >= 10)																//десятичный вид и вывод
			{
				t -= 10;
				i++;
			}
			screen[1][12] = i;
			screen[1][14] = t;
		}	
	
//вывод тока заряда
	if(charge && (u_out >= u_bat))
	{
		if(charge == 1)
			t = ((long)(u_out - u_bat)) * 155 >> 8;
		if(charge == 2)
			t = ((long)(u_out - u_bat)) * 233 >> 8;
		if(charge == 3)
			t = ((long)(u_out - u_bat)) * 388 >> 8;
	
		i_ch = t;
		
		if(t > 99)
		{
			screen[2][11] = 0;
			screen[2][12] = 30;
			screen[2][13] = 26;
			screen[2][14] = 16;	
		}
		else
		{
			i=0;																				//преобразование в двоично-
			while(t >= 10)																//десятичный вид и вывод
			{
				t -= 10;
				i++;
			}
			screen[2][11] = 16;
			screen[2][13] = 19;
			screen[2][12] = i;
			screen[2][14] = t;
		}
	}
	else
	{
		i_ch = 0;
		screen[2][11] = 16;
		screen[2][12] = 0;
		screen[2][13] = 19;
		screen[2][14] = 0;
	}
		
//вывод состояния сервера		
	screen[4][7] = last_code[cur_code];	
	screen[4][8] = dur[cur_code];

	switch(serv_state)
	{
		case OFF_S: //OFF
			screen[4][10] = 16;
			screen[4][11] = 16;
			screen[4][12] = 16;		
			screen[4][13] = 0;
			screen[4][14] = 15;
			screen[4][15] = 15;
			break;
			
		case IDLE: //IDLE
			screen[4][10] = 16;
			screen[4][11] = 16;
			screen[4][12] = 22;
			screen[4][13] = 13;
			screen[4][14] = 23;
			screen[4][15] = 14;
			break;	
			
		case START: //START
			screen[4][10] = 16;
			screen[4][11] = 27;
			screen[4][12] = 28;
			screen[4][13] = 10;
			screen[4][14] = 26;
			screen[4][15] = 28;
			break;
			
		case ACTIVE_S: //ACTIVE
			screen[4][10] = 10;
			screen[4][11] = 12;
			screen[4][12] = 28;
			screen[4][13] = 22;
			screen[4][14] = 30;
			screen[4][15] = 14;
			break;
			
		case STOP: //STOP
			screen[4][10] = 16;
			screen[4][11] = 16;
			screen[4][12] = 27;
			screen[4][13] = 28;
			screen[4][14] = 0;
			screen[4][15] = 25;
			break;
			
		default:
			screen[4][10] = 16;
			screen[4][11] = 16;
			screen[4][12] = 16;
			screen[4][13] = 16;
			screen[4][14] = 16;
			screen[4][15] = 16;	
	}

//вывод состояния батареи
	switch(batt_state)
	{
		case OFF_B: //OFF
			screen[5][8] = 16;
			screen[5][10] = 16;
			screen[5][11] = 16;
			screen[5][12] = 16;		
			screen[5][13] = 0;
			screen[5][14] = 15;
			screen[5][15] = 15;
			break;
			
		case TEST: //TEST
			screen[5][8] = 16;
			screen[5][10] = 16;
			screen[5][11] = 16;
			screen[5][12] = 28;
			screen[5][13] = 14;
			screen[5][14] = 27;
			screen[5][15] = 28;
			break;	
			
		case CHARGE: //CHARGE
			screen[5][8] = charge;
			screen[5][10] = 12;
			screen[5][11] = 21;
			screen[5][12] = 10;
			screen[5][13] = 26;
			screen[5][14] = 20;
			screen[5][15] = 14;
			break;
			
		case ACTIVE_B: //ACTIVE
			screen[5][8] = 16;
			screen[5][10] = 10;
			screen[5][11] = 12;
			screen[5][12] = 28;
			screen[5][13] = 22;
			screen[5][14] = 30;
			screen[5][15] = 14;
			break;
			
		case DISCH: //DISCH
			screen[5][8] = 16;
			screen[5][10] = 16;
			screen[5][11] = 13;
			screen[5][12] = 22;
			screen[5][13] = 27;
			screen[5][14] = 12;
			screen[5][15] = 21;
			break;
			
		case BAD: //BAD
			screen[5][8] = 16;
			screen[5][10] = 16;
			screen[5][11] = 16;
			screen[5][12] = 16;
			screen[5][13] = 11;
			screen[5][14] = 10;
			screen[5][15] = 13;
			break;
			
		default:
			screen[5][8] = 16;
			screen[5][10] = 16;
			screen[5][11] = 16;
			screen[5][12] = 16;
			screen[5][13] = 16;
			screen[5][14] = 16;
			screen[5][15] = 16;	
	}
	
//вывод состояния AC	
	if(ac_ok)
	{
		screen[7][0] = 10;
		screen[7][1] = 12;
		screen[7][2] = 16;
		screen[7][3] = 20;
		screen[7][4] = 0;
		screen[7][5] = 0;
		screen[7][6] = 13;
	}
	else
	{
		screen[7][0] = 10;
		screen[7][1] = 12;
		screen[7][2] = 16;
		screen[7][3] = 15;
		screen[7][4] = 10;
		screen[7][5] = 22;
		screen[7][6] = 23;
	}
	
	//вывод состояния DC	
	if(dc_ok)
	{
		screen[7][9] = 13;
		screen[7][10] = 12;
		screen[7][11] = 16;
		screen[7][12] = 20;
		screen[7][13] = 0;
		screen[7][14] = 0;
		screen[7][15] = 13;
	}		
	else
	{
		screen[7][9] = 13;
		screen[7][10] = 12;
		screen[7][11] = 16;		
		screen[7][12] = 0;
		screen[7][13] = 15;
		screen[7][14] = 15;
		screen[7][15] = 16;
	}

	return;
}
