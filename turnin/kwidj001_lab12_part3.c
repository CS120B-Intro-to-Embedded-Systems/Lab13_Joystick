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
unsigned short inputHor;
task *taskPtr;
unsigned char dir = 0; //0 for left 1, for right


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
      }else if(inputHor > 512){
        state = left;
      }else if(inputHor < 512){
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
      dir = 0;
      if(pattern == 0x80){
        pattern = 0x01
      }else{
        pattern <<= 1;
      }
      break;
    case right:
      dir = 1;
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

enum speedControlStates{sc_wait, leftSpeed, rightSpeed};

//change rightSpeed at intervals [0, 128],[128,256],[256, 384],[384, 512]
//change leftSpeed at intervals [512, 640],[640, 768],[768, 896],[896, 1023]
unsigned long speed[4] = {1000, 500, 250, 100};

int speedControlTick(int state){
  //Transitions
  switch(state){
    case sc_wait:
      if(dir == 0){
        state = leftSpeed;
      }else if(dir == 1){
        state = rightSpeed;
      }else{
        state = wait;
      }
      break;
    case leftSpeed: state = wait; break;
    case rightSpeed: state = wait; break;
    default: state = wait; break;
  }
  //Actions
  switch(state){
    case sc_wait: break;
    case leftSpeed:
      if(inputHor > 512 && inputHor < 640){
        *taskPtr->period = speed[0];
      }else if(inputHor >= 640 && inputHor < 768){
        *taskPtr->period = speed[1];
      }else if(inputHor >= 768 && inputHor < 896){
        *taskPtr->period = speed[2];
      }else if(inputHor >= 896 && inputHor <= 1023){
        *taskPtr->period = speed[3]
      }
      break;
    case rightSpeed:
      if(inputHor > 384 && inputHor < 512){
        *taskPtr->period = speed[0];
      }else if(inputHor > 256 && inputHor <= 384){
        *taskPtr->period = speed[1];
      }else if(inputHor > 128 && inputHor <= 256){
        *taskPtr->period =speed[2];
      }else if(inputHor >= 0 && inputHor <= 128){
        *taskPtr->period = speed[3];
      }
      break;
  }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    //Note: you do not need to set the DDRA to enable the Analog to Digital circuitry
    DDRB = 0xFF; PORTB = 0x00;
    A2D_init();
    inputHor = ADC;
    /* Insert your solution below */
    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);
    taskPtr = &task1;
    const char start = -1;

    //Task1 (shiftTick)
    task1.state = start; //Task initial state
    task1.period = 1000; //task period
    task1.elapsedTime = task1.period;
    task1.TickFct = &shiftTick;

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
