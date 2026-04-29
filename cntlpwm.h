/*
 * cntlpwm.h
 *
 *  Created on: Mar 23, 2026
 *      Author: K
 */

#ifndef CNTLPWM_H_
#define CNTLPWM_H_


#define IDLE         (0)
#define PREPARE      (1)
#define WHITE_DETECT (2)
#define DELAY1       (3)
#define RUN          (4)
#define DELAY2       (5)
#define TURN         (6)
#define STOP         (7)

// ============================================================================
// [ADDED] Project 10 — Additional black line machine states
// ============================================================================
#define BL_START_DELAY  (8)
#define BL_INTERCEPT    (9)
#define BL_INTERCEPT_DELAY (10)
#define BL_TURN         (11)
#define BL_TURN_DELAY   (12)
#define BL_TRAVEL       (13)
#define BL_CIRCLE       (14)
#define BL_CIRCLE_DELAY (15)
#define BL_FOLLOW       (16)
#define BL_EXIT         (17)
#define BL_EXIT_DRIVE   (18)
#define BL_STOP         (19)
// [ADDED] 10-20 second delay between events (500 ticks = 10s at 20ms)------------------------------------------------
#define BL_EVENT_DELAY  (500)
// [ADDED] Time for circle display to show (after ~14s of following)-------------------------------------------------------------------------
#define BL_CIRCLE_SHOW_TIME (700)

// Line Following Direction Memory

#define TURN_RIGHT  (1)
#define TURN_LEFT   (2)

// Follow Status Flags
#define FOLLOW      (1)
#define DONT_FOLLOW (0)

// System Readiness
#define READY       (1)
#define NOT_READY   (0)

extern unsigned int arc_diff;             // [ADDED] Speed difference for arc turns

void All_Movement_Off(void);
void Forward_Fast(void);
void Forward_Med(void);
void Forward_Slow(void);
void Right_Slow(void);
void Left_Slow(void);
void Left_Slow_Reverse(void);
void Right_Slow_Reverse(void);
void Reverse_Slow(void);
void Reverse_Slow2(void);
void Right_Slow_Both (void);
void Right_Slow3_Both(void);
void Left_Slow_Both (void);


// Black Line Intercept State Machine
void black_line_machine(void);
void prepare_case(void);
void white_detect_case(void);
void delay_case_1(void);
void run_case(void);
void delay_case_2(void);
void turn_case(void);
void stop_case(void);

// Line Following State Machine
void line_follow_machine(void);
void follow_prepare_case(void);
void follow_delay_case_1(void);
void follow_run_case(void);
void follow_delay_case_2(void);
void follow_stop_case(void);

// ============================================================================
// [ADDED] Project 10 — Arc turn and exit command helpers
// ============================================================================
void Arc_Right();
void Arc_Left();


extern unsigned int white_value;         // Example: replace with your calibrated value
extern unsigned int black_value;         // Example: replace with your calibrated value


// --- Globals defined in this file ---
extern unsigned char detect_state;       // Starts idle
extern unsigned char follow_state;       // Starts idle
extern unsigned char last_movement;   // Memory of last turn direction
extern char follow_or_not;             // Flag to keep going or stop
extern char ready_to_run_command;   // Flag for next project phases
#endif /* CNTLPWM_H_ */
