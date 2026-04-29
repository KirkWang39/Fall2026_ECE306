/*
 * wheels.c
 *
 *  Created on: Feb 25, 2026
 *      Author: K
 */
#include  "msp430.h"
#include  <string.h>
#include <timer.h>
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"

extern volatile unsigned int wheel_test_timer;

void test_motor(void){
    if(wheel_test_timer<=20){
      Wheels_Forward();
    }
    else {
      Wheels_Stop();
    }
}

void motor_safety_check(void) {
  if ((RIGHT_FORWARD_SPEED > 0) && (RIGHT_REVERSE_SPEED > 0)) {
    P1OUT |= RED_LED;
    RIGHT_FORWARD_SPEED = WHEEL_OFF;
    RIGHT_REVERSE_SPEED = WHEEL_OFF;
    LEFT_FORWARD_SPEED = WHEEL_OFF;
    LEFT_REVERSE_SPEED = WHEEL_OFF;
  }
  if ((LEFT_FORWARD_SPEED > 0) && (LEFT_REVERSE_SPEED > 0)) {
    P1OUT |= RED_LED;
    LEFT_FORWARD_SPEED = WHEEL_OFF;
    LEFT_REVERSE_SPEED = WHEEL_OFF;
    RIGHT_FORWARD_SPEED = WHEEL_OFF;
    RIGHT_REVERSE_SPEED = WHEEL_OFF;
  }
}

void Wheels_Stop(void){
    RIGHT_FORWARD_SPEED = WHEEL_OFF;
    RIGHT_REVERSE_SPEED = WHEEL_OFF;
    LEFT_FORWARD_SPEED = WHEEL_OFF;
    LEFT_REVERSE_SPEED = WHEEL_OFF;
}

void Wheels_Forward(void){
    Wheels_Stop();
    RIGHT_FORWARD_SPEED = SLOW;
    LEFT_FORWARD_SPEED = SLOW;
}



void Wheels_Reverse(void){
    Wheels_Stop();
    RIGHT_REVERSE_SPEED = SLOW;
    LEFT_REVERSE_SPEED = SLOW;
}

void Wheels_Spin_CW(void){
    Wheels_Stop();
    RIGHT_FORWARD_SPEED = WHEEL_OFF;
    LEFT_FORWARD_SPEED = SLOW;
}

void Wheels_Spin_CCW(void){
    Wheels_Stop();
    RIGHT_FORWARD_SPEED = SLOW;
    LEFT_FORWARD_SPEED = WHEEL_OFF;
}
