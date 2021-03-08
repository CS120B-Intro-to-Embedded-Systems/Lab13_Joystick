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
#endif

//Example: unsigned short input = ADC //short is required to store all 10 bits
unsigned char tmpB;
unsigned short input; //for A2D


void A2D_init(){
  ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
  //ADEN: Enables analog-to-digital conversion
  //ADSC: Starts analog-to-digital conversion
  //ADATE: Enables auto-triggering, allowing for constant
  //        analog to digital conversions.
}
int main(void) {
    /* Insert DDR and PORT initializations */
    //Note: you do not need to set the DDRA to enable the Analog to Digital circuitry
    DDRB = 0xFF; PORTB = 0x00;

    A2D_init();
    /* Insert your solution below */
    while (1) {
      input = ADC;
      tmpB = (char)input;
      PORTB = ~tmpB;
    }
    return 1;
}
