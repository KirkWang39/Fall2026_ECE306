/*
 * shapes.c
 *
 *  Created on: Feb 18, 2026
 *      Author: K
 */


#include  "msp430.h"
#include  <string.h>
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"



extern char display_line[4][11];
extern char *display[4];
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display;
extern volatile unsigned int update_display_count;
extern volatile unsigned int Time_Sequence;
extern volatile char one_time;
extern volatile unsigned int ADC_Left_Detect;
extern volatile unsigned int ADC_Right_Detect;
extern volatile unsigned int ADC_Thumb; // We are going to use this for a pro-trick!
extern volatile unsigned int Timer_Counter;
unsigned int proj6_substate = P6_START_DELAY;

//void Run_Straight(void){
// switch(state){
// case WAIT: // Begin
// wait_case();
// break; //
// case START: // Begin
// start_case();
// break; //
// case RUN: // Run
// run_case();
// break; //
// case END: // End
// end_case();
// break; //
// default: break;
// }
//}
//
//void Run_Circle(void){
// switch(state){
// case WAIT: // Begin
// wait_case();
// break; //
// case START: // Begin
// start_case();
// break; //
// case RUN: // Run
// run_case_circle();
// break; //
// case END: // End
// end_case();
// break; //
// default: break;
// }
//}
//
//void Run_Triangle(void){
// switch(state){
// case WAIT: // Begin
// wait_case();
// break; //
// case START: // Begin
// start_case();
// break; //
// case RUN: // Run
// run_case_triangle();
// break; //
// case END: // End
// end_case();
// break; //
// default: break;
// }
//}
//
//void Run_Figure8(void){
// switch(state){
// case WAIT: // Begin
// wait_case();
// break; //
// case START: // Begin
// start_case();
// break; //
// case RUN: // Run
// run_case_figure8();
// break; //
// case END: // End
// end_case();
// break; //
// default: break;
// }
//}
//
//
//void wait_case(void){
// if(time_change){
// time_change = 0;
// if(delay_start++ >= WAITING2START){
// delay_start = 0;
// state = START;
// }
// }
//}
//
//
//void start_case(void){
// cycle_time = 0;
// right_motor_count = 0;
// left_motor_count = 0;
// Forward_On();
// segment_count = 0;
// state = RUN;
//}
//
//
//void run_case(void){
// if(time_change){
// time_change = 0;
// if(segment_count <= TRAVEL_DISTANCE){
// if(right_motor_count++ >= RIGHT_COUNT_TIME){
// P6OUT &= ~R_FORWARD;
// }
// if(left_motor_count++ >= LEFT_COUNT_TIME){
// P6OUT &= ~L_FORWARD;
// }
// if(cycle_time >= WHEEL_COUNT_TIME){
// cycle_time = 0;
// right_motor_count = 0;
// left_motor_count = 0;
// segment_count++;
// Forward_Move();
// }
// }else{
// state = END;
// }
// }
//}
//
//void run_case_circle(void){
// if(time_change){
// time_change = 0;
// if(segment_count <= TRAVEL_DISTANCE_C){
// if(right_motor_count++ >= RIGHT_COUNT_TIME_C){
// P6OUT &= ~R_FORWARD;
// }
// if(left_motor_count++ >= LEFT_COUNT_TIME_C){
// P6OUT &= ~L_FORWARD;
// }
// if(cycle_time >= WHEEL_COUNT_TIME_C){
// cycle_time = 0;
// right_motor_count = 0;
// left_motor_count = 0;
// segment_count++;
// Forward_Move();
// }
// }else{
// state = END;
// }
// }
//}
//
//void run_case_figure8(void){
// if(time_change){
// time_change = 0;
// switch(step){
// case 0:
//     travel_distance = TRAVEL_DISTANCE_FL;
//     rightcounttime = RIGHT_COUNT_TIME_FL;
//     leftcounttime = LEFT_COUNT_TIME_FL;
//     break;
//
// case 1:
//     travel_distance = TRAVEL_DISTANCE_FR;
//     rightcounttime = RIGHT_COUNT_TIME_FR;
//     leftcounttime = LEFT_COUNT_TIME_FR;
//     break;
//
// case 2:
//     state = END;
//     break;
// }
//
// if(segment_count <= travel_distance){
// if(right_motor_count++ >= rightcounttime){
// P6OUT &= ~R_FORWARD;
// }
// if(left_motor_count++ >= leftcounttime){
// P6OUT &= ~L_FORWARD;
// }
// if(cycle_time >= WHEEL_COUNT_TIME_FL){
// cycle_time = 0;
// right_motor_count = 0;
// left_motor_count = 0;
// segment_count++;
// Forward_Move();
// }
// }else{
// segment_count = 0;
// step++;
// }
// }
//}
//
//void run_case_triangle(void){
// if(time_change){
// time_change = 0;
// switch(stept){
// case 0:
//     travel_distance = TRAVEL_DISTANCE_TS;
//     rightcounttime = RIGHT_COUNT_TIME_TS;
//     leftcounttime = LEFT_COUNT_TIME_TS;
//     break;
//
// case 1:
//     travel_distance = TRAVEL_DISTANCE_TT;
//     rightcounttime = RIGHT_COUNT_TIME_TT;
//     leftcounttime = LEFT_COUNT_TIME_TT;
//     break;
// case 2:
//     travel_distance = TRAVEL_DISTANCE_TS;
//     rightcounttime = RIGHT_COUNT_TIME_TS;
//     leftcounttime = LEFT_COUNT_TIME_TS;
//     break;
// case 3:
//     travel_distance = TRAVEL_DISTANCE_TT;
//     rightcounttime = RIGHT_COUNT_TIME_TT;
//     leftcounttime = LEFT_COUNT_TIME_TT;
//     break;
// case 4:
//     travel_distance = TRAVEL_DISTANCE_TS;
//     rightcounttime = RIGHT_COUNT_TIME_TS;
//     leftcounttime = LEFT_COUNT_TIME_TS;
//     break;
// case 5:
//     travel_distance = TRAVEL_DISTANCE_TT;
//     rightcounttime = RIGHT_COUNT_TIME_TT;
//     leftcounttime = LEFT_COUNT_TIME_TT;
//     break;
// case 6:
//     travel_distance = TRAVEL_DISTANCE_TS;
//     rightcounttime = RIGHT_COUNT_TIME_TS;
//     leftcounttime = LEFT_COUNT_TIME_TS;
//     break;
// case 7:
//     travel_distance = TRAVEL_DISTANCE_TT;
//     rightcounttime = RIGHT_COUNT_TIME_TT;
//     leftcounttime = LEFT_COUNT_TIME_TT;
//     break;
// case 8:
//     travel_distance = TRAVEL_DISTANCE_TS;
//     rightcounttime = RIGHT_COUNT_TIME_TS;
//     leftcounttime = LEFT_COUNT_TIME_TS;
//     break;
// case 9:
//     travel_distance = TRAVEL_DISTANCE_TT;
//     rightcounttime = RIGHT_COUNT_TIME_TT;
//     leftcounttime = LEFT_COUNT_TIME_TT;
//     break;
// case 10:
//     travel_distance = TRAVEL_DISTANCE_TS;
//     rightcounttime = RIGHT_COUNT_TIME_TS;
//     leftcounttime = LEFT_COUNT_TIME_TS;
//     break;
// case 11:
//     state = END;
//     break;
// }
//
// if(segment_count <= travel_distance){
// if(right_motor_count++ >= rightcounttime){
// P6OUT &= ~R_FORWARD;
// }
// if(left_motor_count++ >= leftcounttime){
// P6OUT &= ~L_FORWARD;
// }
// if(cycle_time >= WHEEL_COUNT_TIME_T){
// cycle_time = 0;
// right_motor_count = 0;
// left_motor_count = 0;
// segment_count++;
// Forward_Move();
// }
// }else{
// segment_count = 0;
// stept++;
// }
// }
//}
//

extern volatile unsigned int ADC_Left_Detect;
extern volatile unsigned int ADC_Right_Detect;
void Run_Project6(void) {
    unsigned int threshold = ADC_Thumb;

    switch(proj6_substate) {
        case P6_START_DELAY:
            Wheels_Stop();
            if (Timer_Counter >= 20) { // 1.0s delay (assuming 50ms ticks)
                proj6_substate = P6_MOVE_TO_LINE;
            }
            break;

        case P6_MOVE_TO_LINE:
            // Move forward until black line is detected
            if (ADC_Left_Detect > threshold || ADC_Right_Detect > threshold) {
                Wheels_Stop(); // Stop immediately!
                strcpy(display_line[3], "BLACK LINE");
                display_changed = TRUE;
                Timer_Counter = 0;
                proj6_substate = P6_STOP_ON_LINE; // SKIP THE NUDGE
            } else {
                Wheels_Forward();
            }
            break;

        case P6_STOP_ON_LINE:
            Wheels_Stop();
            if (Timer_Counter >= 60) { // 3.0s pause
                proj6_substate = P6_ALIGN_TO_LINE;
            }
            break;

        case P6_ALIGN_TO_LINE:
            // Requirement: Turn so detectors end up above the black line
            if (ADC_Left_Detect > threshold) {
                Wheels_Stop();
                proj6_substate = P6_FINISHED;
            } else {
                // SOFTWARE PWM PULSING (STUTTER MODE)
                if (Timer_Counter % 16 == 0) {
                    Wheels_Spin_CW(); // Turn ON for 50ms
                } else {
                    Wheels_Stop();    // Turn OFF for 150ms
                }
            }
            break;

        case P6_FINISHED:
            Wheels_Stop();
            HEXtoBCD(ADC_Left_Detect);
            adc_line(4, 0);
            break;
    }
}

//void end_case(void){
// Forward_Off();
// state = WAIT;
// event = NONE;
//}
//
//void Forward_On(void){
//    P6OUT |= R_FORWARD;
//    P6OUT |= L_FORWARD;
//}
//void Forward_Move(void){
//    P6OUT |= R_FORWARD;
//    P6OUT |= L_FORWARD;
//}
//void Forward_Off(void){
//    P6OUT &= ~R_FORWARD;
//    P6OUT &= ~L_FORWARD;
//}
//
//void Wheels_Left_Turn(void) {
//    Wheels_Stop();          // Always stop first for safety!
//
//    // To turn Left, the Right motor drives Forward, and the Left motor stays Off.
//    P6OUT |= R_FORWARD;     // Turn on Right Forward pin
//    P6OUT &= ~L_FORWARD;    // Ensure Left Forward pin is off
//}
//
////-----------------------------------------------------------------------------
//// Smooth Right Turn (Pivots on the right wheel)
////-----------------------------------------------------------------------------
//void Wheels_Right_Turn(void) {
//    Wheels_Stop();          // Always stop first for safety!
//
//    // To turn Right, the Left motor drives Forward, and the Right motor stays Off.
//    P6OUT |= L_FORWARD;     // Turn on Left Forward pin
//    P6OUT &= ~R_FORWARD;    // Ensure Right Forward pin is off
//}

