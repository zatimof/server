#include "..\server.h"

extern unsigned char test, charge, ac_ok, blink, last_code[], cur_code, dur[], press;
extern unsigned char u_bat, u_out, i_ch, user_off;
extern enum {OFF_S, IDLE, START, ACTIVE_S, STOP} serv_state;
extern enum {OFF_B, TEST, CHARGE, ACTIVE_B, DISCH, BAD} batt_state;
extern unsigned int active_cnt;

void logic(void)
{
	switch(batt_state)
	{
		case OFF_B: //OFF
			if(u_bat > BAT_OFF_LEVEL)
				batt_state = CHARGE;
			
			break;
			
		case TEST: //TEST
			if(u_bat < BAT_OFF_LEVEL)
				batt_state = OFF_B;
			
			break;	
			
		case CHARGE: //CHARGE
			if(u_bat < BAT_OFF_LEVEL)
				batt_state = OFF_B;
			
			if((u_bat > BAT_OFF_LEVEL) && (u_bat < BAT_CH12_LEVEL))
			{
				charge = 1;
				PORTB &= 0xFB;
				PORTB |= 0x02;			
			}
			
			if((u_bat >= BAT_CH12_LEVEL) && (u_bat < BAT_CH23_LEVEL))
			{
				charge = 2;
				PORTB &= 0xFD;
				PORTB |= 0x04;			
			}
			
			if((u_bat >= BAT_CH23_LEVEL) && (u_bat < BAT_NOM_LEVEL))
			{
				charge = 3;
				PORTB |= 0x06;
			}
			
			if(u_bat >= BAT_NOM_LEVEL)
			{
				charge = 0;
				PORTB &= 0xFB;
				PORTB |= 0x02;	
				batt_state = ACTIVE_B;				
			}
			
			if(!ac_ok)
			{
				PORTB &= 0xF8;
				batt_state = DISCH;				
			}
			
			break;
			
		case ACTIVE_B: //ACTIVE
			if(u_bat > BAT_MAX_LEVEL)
				PORTB &= 0xF9;
			
			if(u_bat < BAT_MAX_LEVEL)			
				PORTB |= 0x02;
			
			if(u_bat < BAT_OFF_LEVEL)
				batt_state = OFF_B;
			
			if(u_bat < BAT_NOM_LEVEL)
				batt_state = CHARGE;
			
			if(!ac_ok)
			{
				PORTB &= 0xF8;
				batt_state = DISCH;				
			}
			
			break;
			
		case DISCH: //DISCH
			charge = 0;
			
			if(u_bat < BAT_OFF_LEVEL)
				batt_state = OFF_B;
			
			if(ac_ok)
			{
				PORTB |= 0x01;
				batt_state = CHARGE;	
			}
			
			if(blink == 255)
				BEEP_ON;
			
			if(blink == 0)
				BEEP_OFF;
			
			break;
			
		case BAD: //BAD
			if(blink == 128)
				BEEP_ON;
			
			if(blink == 0)
				BEEP_OFF;
			break;
			
		default:
			break;
	}
	
	switch(serv_state)
	{
		case OFF_S: //OFF
	
			FAN_OFF;
			
			if(((last_code[1] == 6) && (last_code[0] == 4)) || ((last_code[0] == 6) && (last_code[1] == 4)))
				if((dur[0] == 1) && (dur[1] == 1))
					serv_state = IDLE;
			
			if(((last_code[1] == 6) && (last_code[0] == 2)) || ((last_code[0] == 6) && (last_code[1] == 2)))
				if(((dur[0] == 1) && (dur[1] == 1)) || ((dur[0] == 2) && (dur[1] == 2)))
					serv_state = IDLE;
			
			if(((last_code[1] == 3) && (last_code[0] == 7)) || ((last_code[0] == 3) && (last_code[1] == 7)))
				if((dur[0] == 2) && (dur[1] == 2))
					serv_state = ACTIVE_S;
			break;
			
		case 1: //IDLE
			
			FAN_OFF;
			
			if(last_code[cur_code] == 0)
				serv_state = OFF_S;
			
			if(((last_code[1] == 6) && (last_code[0] == 2)) || ((last_code[0] == 6) && (last_code[1] == 2)))
				if((dur[0] == 2) && (dur[1] == 2) && ac_ok && (active_cnt > 1000) && (!user_off))			
				{
					serv_state = START;
					press = 255;
				}
			
			if(((last_code[1] == 3) && (last_code[0] == 7)) || ((last_code[0] == 3) && (last_code[1] == 7)))
				if((dur[0] == 2) && (dur[1] == 2))
					serv_state = ACTIVE_S;
			
			break;	
			
		case 2: //START
			
			FAN_ON;
			
			if(last_code[cur_code] == 0)
				serv_state = OFF_S;
			
			if(((last_code[1] == 3) && (last_code[0] == 7)) || ((last_code[0] == 3) && (last_code[1] == 7)))
				if((dur[0] == 2) && (dur[1] == 2))
				{
					serv_state = ACTIVE_S;
					active_cnt = 0;
				}
			break;
			
		case 3: //ACTIVE
			
			FAN_ON;
			
			if(last_code[cur_code] == 0)
				serv_state = OFF_S;
			
			if((u_bat < BAT_LOW_LEVEL) && (!ac_ok) && (active_cnt > 10000))
			{
				serv_state = STOP;
				press = 255;
				user_off = 0;
			}			
			
			if(((last_code[1] == 2) && (last_code[0] == 6)) || ((last_code[0] == 2) && (last_code[1] == 6)))
				if((dur[0] == 2) && (dur[1] == 2))
				{
					serv_state = IDLE;
					active_cnt = 0;
					user_off = 1;
				}
			break;
			
		case 4: //STOP
			if(last_code[cur_code] == 0)
				serv_state = OFF_S;
			
			if(((last_code[1] == 2) && (last_code[0] == 6)) || ((last_code[0] == 2) && (last_code[1] == 6)))
				if((dur[0] == 2) && (dur[1] == 2))
				{
					serv_state = IDLE;
					active_cnt = 0;
				}
			break;
			
		default:
			break;
	}
	return;
}