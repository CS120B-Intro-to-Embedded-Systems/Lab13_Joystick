/*	Author: lab
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #13  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "scheduler.h"
#include "timer.h"
#endif

//Example: unsigned short input = ADC //short is required to store all 10 bits
unsigned char tmpB;
unsigned short input; //for A2D
unsigned short inputHor


void A2D_init(){
  ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
  //ADEN: Enables analog-to-digital conversion
  //ADSC: Starts analog-to-digital conversion
  //ADATE: Enables auto-triggering, allowing for constant
  //        analog to digital conversions.
}

enum shiftStates{wait, left, right};
int shiftTick(int state){
  //Local Variables
  static unsigned char pattern = 0x80;
  static unsigned char row = 0xFE;

  //Transitions
  switch(state){
    case wait:
      if(inputHor == 512){
        state = wait;
      }else if(inputHor < 512){
        state = left;
      }else if(inputHor > 512){
        state = right;
      }
      break;
    case left: state = wait; break;
    case right: state = wait; break;
    default: state = wait; break;
  }
  //Action
  switch(state){
    case wait: break;
    case left:
      if(pattern == 0x80){
        pattern = 0x01
      }else{
        pattern <<= 1;
      }
      break;
    case right:
      if(pattern == 0x01){
        pattern = 0x80;
      }else{
        pattern >>= 1;
      }
  }
  PORTC = pattern;
  PORTD = row;
  return state;
}
int main(void) {
    /* Insert DDR and PORT initializations */
    //Note: you do not need to set the DDRA to enable the Analog to Digital circuitry
    DDRB = 0xFF; PORTB = 0x00;
    inputHor = ADC; //A0 = L/R
    //inputVer = ~PINA & 0x02; //A1 = U/D
    A2D_init();
    /* Insert your solution below */
    static task task1 task2;
    task *tasks[] = {&task1, &task2};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);

    const char start = -1;

    //Task1 (shiftTick)
    task1.state = start; //Task initial state
    task1.period = 100; //task period
    task1.elapsedTime = task1.period;
    task1.TickFct = &shiftTick;
    //Task2 (speedControlTick)
    task2.state = start;
    task2.period = 100;
    task2.elapsedTime = task2.period;
    task2.TickFct = &speedControlTick;

    unsigned long GCD = tasks[0]->period;
    for(int i = 1; i < numTasks; i++){
      GCD = findGCD(GCD, tasks[i]->period);
    }

    //Set timer and turn on
    TimerSet(GCD);
    TimerOn();

    unsigned short i;
    while (1) {
      for(i = 0; i < numTasks; i++){
        if(tasks[i]->elapsedTime == tasks[i]->period){
          tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
          tasks[i]->elapsedTime = 0;
        }
        tasks[i]->elapsedTime += GCD;
      }
      while(!TimerFlag);
      TimerFlag = 0;
    }
    return 0;
}
