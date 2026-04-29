/*
 * macros.h
 *
 *  Created on: Feb 5, 2026
 *      Author: K
 */

#ifndef MACROS_H_
#define MACROS_H_

#define ALWAYS                  (1)
#define RESET_STATE             (0)
#define RED_LED              (0x01) // RED LED 0
#define GRN_LED              (0x40) // GREEN LED 1
#define TEST_PROBE           (0x01) // 0 TEST PROBE
#define TRUE                 (0x01) //



//
//#define WHEEL_COUNT_TIME (10)
//#define RIGHT_COUNT_TIME (7)
//#define LEFT_COUNT_TIME (8)
//#define TRAVEL_DISTANCE (2)
//
//#define WHEEL_COUNT_TIME_C (10)
//#define RIGHT_COUNT_TIME_C (1)
//#define LEFT_COUNT_TIME_C (10)
//#define TRAVEL_DISTANCE_C (200)
//
//#define WHEEL_COUNT_TIME_FL (10)
//#define RIGHT_COUNT_TIME_FL (1)
//#define LEFT_COUNT_TIME_FL (10)
//#define TRAVEL_DISTANCE_FL (175)
//
//#define WHEEL_COUNT_TIME_FR (10)
//#define RIGHT_COUNT_TIME_FR (10)
//#define LEFT_COUNT_TIME_FR (1)
//#define TRAVEL_DISTANCE_FR (175)
//
//#define WHEEL_COUNT_TIME_T (20)
//
//#define RIGHT_COUNT_TIME_TS (20)
//#define LEFT_COUNT_TIME_TS (20)
//#define TRAVEL_DISTANCE_TS (10)
//
//#define RIGHT_COUNT_TIME_TT (30)
//#define LEFT_COUNT_TIME_TT (0)
//#define TRAVEL_DISTANCE_TT (10)

#define WAITING2START (50)
#define PRESSED (0)
#define RELEASED (1)
#define OKAY (1)
#define NOT_OKAY (0)
#define DEBOUNCE_TIME (10000)
#define DEBOUNCE_RESTART (0)

unsigned int Last_Time_Sequence;
unsigned int cycle_time;
unsigned int time_change;

unsigned int left_motor_count;
unsigned int right_motor_count;
unsigned int okay_to_look_at_switch1;
unsigned int okay_to_look_at_switch2;
unsigned int segment_count;
unsigned int sw1_position;
unsigned int sw2_position;
unsigned int state;
unsigned int delay_start;
volatile int count_debounce_SW1;
volatile int count_debounce_SW2;
char event;
unsigned int switch_count;

unsigned int step;
unsigned int stept;
unsigned int travel_distance;
unsigned int rightcounttime;
unsigned int leftcounttime;


#define P6_START_DELAY   (0)
#define P6_MOVE_TO_LINE  (1)
#define P6_STOP_ON_LINE  (2)
#define P6_ALIGN_TO_LINE (3)
#define P6_FINISHED      (4)

//////// IOT BS
#define BEGINNING       (0)
#define RING_BUF_SIZE   (256)  //changed from 16 to 64 for project9
#define BAUD_115200     (1)
#define BAUD_460800     (2)
#define BAUD_57600 (3)


#define WAIT        (0)
#define RECEIVED    (1)
#define TRANSMIT    (2)




#endif /* MACROS_H_ */
