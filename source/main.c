/*	Author: huryan18
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

enum States {start, wait, inc, dec, reset} state;
unsigned char tmpB;
unsigned char count = 0;

void TimerOn(){
      TCCR1B = 0x0B;    //bit3 = 0: clear timer on compare; 8MHz clock = 8000000/64 = 125,000 ticks/s
      OCR1A = 125;      //Timer interupt generated when TCNT1 == OCR1A; We want 1ms tick, so 125000 * .001 = 125
      TIMSK1 = 0x2;     //enables interupt for compare matched
      TCNT1 = 0;        //init avr counter
      _avr_timer_cntcurr = _avr_timer_M;
      SREG |= 0x80;
}
void TimerOff(){
      TCCR1B = 0x00;
}
void TimerISR(){
      TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect){
      _avr_timer_cntcurr--;
      if(_avr_timer_cntcurr == 0){
            TimerISR();
            _avr_timer_cntcurr = _avr_timer_M;
      }
}
void TimerSet(unsigned long M){
      _avr_timer_M = M;
      _avr_timer_cntcurr = _avr_timer_M;
}

void tick(){
      switch(state){
            case start:
                  tmpB = 0x07;
                  state = wait;
                  count = 0;
                  break;
            case wait:
                  if(~PINA & 0x01 && ~PINA & 0x02){
                        tmpB = 0;
                        state = reset;
                  }
                  else if(~PINA & 0x01){
                        if(tmpB < 9)
                              tmpB++;
                        count = 0;
                        state = inc;
                  }
                  else if(~PINA & 0x02){
                        if(tmpB > 0)
                              tmpB--;
                        count = 0;
                        state = dec;
                  }
                  else
                        state = wait;
                  break;
            case inc:
                  if(~PINA & 0x02){
                        tmpB = 0;
                        state = reset;
                  }
                  if(~PINA & 0x01)
                        state = inc;
                  else
                        state = wait;
                  break;
            case dec:
                  if(~PINA & 0x01){
                        tmpB = 0;
                        state = reset;
                  }
                  if(~PINA & 0x02)
                        state = dec;
                  else
                        state = wait;
                  break;
            case reset:
                  if(~PINA & 0x01 || ~PINA & 0x02)
                        state = reset;
                  else
                        state = wait;
                  break;
      }
      switch(state){
            case inc:
                  if(count == 10){
                        if(tmpB < 9)
                              tmpB++;
                        count = 0;
                  }
                  else
                        count++;
                  break;
            case dec:
                  if(count == 10){
                        if(tmpB > 0)
                              tmpB--;
                        count = 0;
                  }
                  else
                        count++;
                  break;
            case reset:
                  tmpB = 0;
                  break;
      }
}

int main(void) {
      /* Insert DDR and PORT initializations */
      DDRA = 0x00;      PORTA = 0xff;
      DDRB = 0xFF;      PORTB = 0x00;
      DDRC = 0xff;      PORTC = 0x00;
      DDRD = 0xff;      PORTD = 0x00;
      LCD_init();
      TimerSet(100);
      TimerOn();
      tmpB = 0x00;
      count = 0;
      // LCD_DisplayString(1, "Hello World");
      /* Insert your solution below */
      state = start;
      while (1) {
            tick();
            PORTB = tmpB;
			LCD_Cursor(1);
            LCD_WriteData(tmpB + '0');
            while(!TimerFlag);
            TimerFlag = 0;
            continue;
      }
      return 1;
}
