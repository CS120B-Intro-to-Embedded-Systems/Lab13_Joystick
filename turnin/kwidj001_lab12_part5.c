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
unsigned char inputHor, inputVer;
task *taskPtr;
unsigned char dir = 0; //0 for left ,
                        //1 for right,
                        //2 for up,
                        //3 for down
                        //4 for upLeft
                        //5 for upRight
                        //6 for downLeft
                        //7 for downRight


void A2D_init(){
  ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
  //ADEN: Enables analog-to-digital conversion
  //ADSC: Starts analog-to-digital conversion
  //ADATE: Enables auto-triggering, allowing for constant
  //        analog to digital conversions.
}
//Pins on PORTA are used as input for A2D conversion
//  The default cahnnel is 0 (PA0)
//The value of pinNum determines the pin on PORTA
//  used for A2D conversion
//Valid values range between 0 and 7, where the value
//  represents the desired pin for A2D conversion
void Set_A2D_Pin(unsigned char pinNum){
  ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
  //Allow channel to stabilize
  static unsigned char i = 0;
  for(i = 0; i < 15; i++){asm("nop");}
}
enum shiftStates{wait, left, right, up, down, upLeft, upRight, downLeft, downRight};
int shiftTick(int state){
  //Local Variables
  static unsigned char pattern = 0x80;
  static unsigned char row = 0xFE;

  //Transitions
  switch(state){
    case wait:
      if(inputHor == 512 && inputVer == 512){
        state = wait;
      }else if(inputHor > 512 && inputVer == 512){
        state = left;
      }else if(inputHor < 512 && inputVer == 512){
        state = right;
      }else if(inputVer > 512 && inputHor == 512){
        state = up;
      }else if(inputVer < 512 && inputHor == 512){
        state = down;
      }else if(inputHor > 512 && inputVer > 512){
        state = upLeft;
      }else if(inputHor < 512 && inputVer > 512){
        state = upRight;
      }else if(inputHor > 512 && inputVer < 512){
        state = downLeft;
      }else if(inputHor < 512 && inputVer < 512){
        state = downRight;
      }
      break;
    case left: state = wait; break;
    case right: state = wait; break;
    case up: state = wait; break;
    case down: state = wait; break;
    case upLeft: state = wait; break;
    case upRight: state = wait; break;
    case downLeft: state = wait; break;
    case downRight: state = wait; break;
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
      break;
    case up:
      dir = 2
      if(row == 0xFE){
        row = 0xEF;
      }else{
        row >>= 1;
      }
      break;
    case down:
      dir = 3;
      if(row == 0xEF){
        row = 0xFE;
      }else{
        row <<= 1;
      }
      break;
    case upLeft:
      dir = 4;
      if(pattern == 0x80 && row == 0xFE){
        pattern = 0x01;
        row = 0xEF;
      }else{
        pattern <<= 1;
        row >>= 1;
      }
      break;
    case upRight:
      dir = 5;
      if(pattern == 0x01 && row == 0xFE){
        pattern = 0x80;
        row = 0xEF;
      }else{
        pattern >>= 1;
        row >>= 1;
      }
      break;
    case downLeft:
      dir = 6;
      if(pattern == 0x80 && row == 0xEF){
        pattern = 0x01;
        row = 0xFE;
      }else{
        pattern <<= 1;
        row <<= 1;
      }
      break;
    case downRight:
      dir = 7;
      if(pattern == 0x01 && row == 0xEF){
        pattern = 0x80;
        row = 0xFE;
      }else{
        pattern >>= 1;
        row <<= 1;
      }
      break;
    default: break;

  }
  PORTC = pattern;
  PORTD = row;
  return state;
}
//FOR VARIABLE SPEED
enum xSpeedControlStates{sc_wait, leftSpeed, rightSpeed};

//change rightSpeed at intervals [0, 128],[128,256],[256, 384],[384, 512]
//change leftSpeed at intervals [512, 640],[640, 768],[768, 896],[896, 1023]
unsigned long speed[4] = {1000, 500, 250, 100};

int xSpeedControlTick(int state){
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

//FOR VARIABLE SPEED
enum ySpeedControlStates{sc_wait, upSpeed, downSpeed};

//change rightSpeed at intervals [0, 128],[128,256],[256, 384],[384, 512]
//change leftSpeed at intervals [512, 640],[640, 768],[768, 896],[896, 1023]
unsigned long speed[4] = {1000, 500, 250, 100};

int ySpeedControlTick(int state){
  //Transitions
  switch(state){
    case sc_wait:
      if(dir == 2){
        state = upSpeed;
      }else if(dir == 3){
        state = downSpeed;
      }else{
        state = wait;
      }
      break;
    case upSpeed: state = wait; break;
    case downSpeed: state = wait; break;
    default: state = wait; break;
  }
  //Actions
  switch(state){
    case sc_wait: break;
    case upSpeed:
      if(inputVer > 512 && inputVer < 640){
        *taskPtr->period = speed[0];
      }else if(inputVer >= 640 && inputVer < 768){
        *taskPtr->period = speed[1];
      }else if(inputVer >= 768 && inputVer < 896){
        *taskPtr->period = speed[2];
      }else if(inputVer >= 896 && inputVer <= 1023){
        *taskPtr->period = speed[3]
      }
      break;
    case downSpeed:
      if(inputVer > 384 && inputVer < 512){
        *taskPtr->period = speed[0];
      }else if(inputVer > 256 && inputVer <= 384){
        *taskPtr->period = speed[1];
      }else if(inputVer > 128 && inputVer <= 256){
        *taskPtr->period =speed[2];
      }else if(inputVer >= 0 && inputVer <= 128){
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
    Set_A2D_Pin(0x02); //U/D on PA1 (L/R default PA0)
    inputHor = ADC;
    inputVer = ~PINA & 0x02; //A1 = U/D
    /* Insert your solution below */
    static task task1, task2, task3;
    task *tasks[] = {&task1, &task2, &task3};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);
    taskPtr = &task1;
    const char start = -1;

    //Task1 (shiftTick)
    task1.state = start; //Task initial state
    task1.period = 1000; //task period
    task1.elapsedTime = task1.period;
    task1.TickFct = &shiftTick;
    //Task2 (xSpeedControlTick)
    task2.state = start;
    task2.period = 1000;
    task2.elapsedTime = task2.period;
    task2.TickFct = &xSpeedControlTick;
    //Task3 (ySpeedControlTick)
    task3.state = start;
    task3.period = 1000;
    task3.elapsedTime = task3.period;
    task3.TickFct = &ySpeedControlTick;

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
