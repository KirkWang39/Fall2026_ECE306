/*
 * cntlpwm.c
 *
 *  Created on: Mar 23, 2026
 *      Author: K
 */

#include <stdlib.h>
#include  "msp430.h"
#include  <string.h>
#include <timer.h>
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"
#include "adc.h"
#include "cntlpwm.h"
#include "dac.h"
#include "serial.h"

extern char run_timer;




// --- Globals defined in this file ---
unsigned char detect_state = IDLE;       // Starts idle
unsigned char follow_state = IDLE;       // Starts idle
unsigned char last_movement = FORWARD;   // Memory of last turn direction
char follow_or_not = FOLLOW;             // Flag to keep going or stop
char ready_to_run_command = NOT_READY;   // Flag for next project phases

// --- Calibration Values ---
// Note: If these are defined in your ADC file, change them to 'extern' here!
unsigned int white_value = 66;         // Example: replace with your calibrated value
unsigned int black_value = 500;         // Example: replace with your calibrated value
//unsigned int white_value = 200;         // Example: replace with your calibrated value
//unsigned int black_value = 500;         // Example: replace with your calibrated value

// --- External variables (Defined in other files) ---
extern volatile unsigned int Timer_Counter;      // From timer.c (ensure it ticks every 20ms!)
extern volatile unsigned int ADC_Left_Detect;   // From adc.c
extern volatile unsigned int ADC_Right_Detect;  // From adc.c

extern char display_line[4][11];                // From display.c
extern volatile unsigned char display_changed;  // From display.c


// ============================================================================
// [ADDED] Project 10 — Globals for the full autonomous sequence
// ============================================================================
unsigned int bl_timer = 0;                // Timer for black line delays (incremented by Timer_Counter)
unsigned int bl_timer_last = 0;           // [ADDED] Tracks last Timer_Counter to detect ticks
unsigned int circle_timer = 0;            // Timer to detect when car enters circle
unsigned int exit_drive_time = 250;       // [ADDED] How long to drive on exit (default 5s = 250*20ms)
unsigned int arc_right_speed = SLOW;      // [ADDED] Arc right motor speed
unsigned int arc_left_speed = SLOW;       // [ADDED] Arc left motor speed
unsigned int arc_diff = 5000;             // [ADDED] Speed difference for arc (cmd_time_val sets this)



// Timed movement state machine
// ============================================================================
// 1. BLACK LINE INTERCEPT MACHINE
// ============================================================================
// ============================================================================
// 1. STREAMLINED BLACK LINE INTERCEPT MACHINE
// ============================================================================

// ============================================================================
// [MODIFIED] Project 10 — Full Autonomous Black Line Machine
// Matches all Project 10 display and timing requirements
// Triggered by IoT command 'I' (Intercept)
// ============================================================================
void black_line_machine(void){
    switch (detect_state){
        case IDLE: break;

        // --------------------------------------------------------------
        // Step 1: "BL Start" — display message, wait 10-20s, then drive
        // --------------------------------------------------------------
        case BL_START_DELAY:
            All_Movement_Off();
            P2OUT |= IR_LED;
            LCD_BACKLITE_DIMING = PERCENT_80;
            strcpy(display_line[0], " BL Start ");
            display_changed = TRUE;
            Timer_Counter = 0;              // [FIX] Use hardware timer, not bl_timer
            detect_state = BL_INTERCEPT;
            break;

        case BL_INTERCEPT:
            // Wait 10s for TA to see "BL Start", then drive forward
            if (Timer_Counter >= BL_EVENT_DELAY) {
                Forward_Slow();
                detect_state = BL_INTERCEPT_DELAY;
            }
            break;

        // --------------------------------------------------------------
        // Step 2: "Intercept" — detect line, stop, wait 10-20s
        // --------------------------------------------------------------
        case BL_INTERCEPT_DELAY:
            if ((ADC_Left_Detect >= black_value) || (ADC_Right_Detect >= black_value)){
                All_Movement_Off();
                strcpy(display_line[0], "Intercept ");
                display_changed = TRUE;
                Timer_Counter = 0;          // [FIX] Reset hardware timer
                detect_state = BL_TURN;
            }
            break;

        case BL_TURN:
            // Wait 10s for TA to see "Intercept"
            if (Timer_Counter >= BL_EVENT_DELAY) {
                strcpy(display_line[0], " BL Turn  ");
                display_changed = TRUE;
                Left_Slow_Both();
                Timer_Counter = 0;
                detect_state = BL_TURN_DELAY;
            }
            break;

        // --------------------------------------------------------------
        // Step 3: "BL Turn" — turn ~90 degrees, stop, wait 10-20s
        // --------------------------------------------------------------
        case BL_TURN_DELAY:
            Right_Slow_Both();
            if (Timer_Counter >= 40) {      // ~1.5s turning at 20ms — TUNE THIS! --------------------------------------------------------------------
                All_Movement_Off();
                Timer_Counter = 0;
                detect_state = BL_TRAVEL;
            }
            break;

        // --------------------------------------------------------------
        // Step 4: "BL Travel" — display, wait 10-20s, then start following
        // --------------------------------------------------------------
        case BL_TRAVEL:
            All_Movement_Off();
            strcpy(display_line[0], " BL Travel");
            display_changed = TRUE;
            Timer_Counter = 0;
            detect_state = BL_CIRCLE;
            break;

        case BL_CIRCLE:
            // Wait 10s for TA to see "BL Travel", then start following
            if (Timer_Counter >= BL_EVENT_DELAY) {
                circle_timer = 0;
                Timer_Counter = 0;
                follow_or_not = FOLLOW;
                follow_state = PREPARE;     // Hand off to line follower
                detect_state = BL_CIRCLE_DELAY;
            }
            break;

        // --------------------------------------------------------------
        // Step 5: "BL Circle" — display after following for a while
        // --------------------------------------------------------------
        case BL_CIRCLE_DELAY:
            // circle_timer increments with Timer_Counter in this state
            if (Timer_Counter >= BL_CIRCLE_SHOW_TIME) {
                All_Movement_Off();
                follow_state = IDLE;        // Pause following
                strcpy(display_line[0], " BL Circle");
                display_changed = TRUE;
                Timer_Counter = 0;
                detect_state = BL_FOLLOW;
            }
            break;

        case BL_FOLLOW:
            // Wait 10s for TA to see "BL Circle", then resume following
            if (Timer_Counter >= BL_EVENT_DELAY) {
                follow_or_not = FOLLOW;
                follow_state = PREPARE;     // Resume line following
                detect_state = IDLE;        // Machine done — follows until exit cmd
            }
            break;

        // ==============================================================
        // [ADDED] Exit sequence — triggered by IoT 'E' command
        // ==============================================================
        case BL_EXIT:
            follow_state = IDLE;
            follow_or_not = DONT_FOLLOW;
            All_Movement_Off();
            strcpy(display_line[0], " BL Exit  ");
            display_changed = TRUE;
            Timer_Counter = 0;
            detect_state = BL_EXIT_DRIVE;
            break;

        case BL_EXIT_DRIVE:
            if (Timer_Counter < BL_EVENT_DELAY) {
                All_Movement_Off();                     // Stay stopped during delay
            }
            else if (Timer_Counter < (BL_EVENT_DELAY + exit_drive_time)) {
                Forward_Slow_Tweak();                         // Drive away from circle
            }
            else {
                All_Movement_Off();
                strcpy(display_line[0], " BL Stop  ");
                strcpy(display_line[1], "  Project ");
                strcpy(display_line[2], " Complete!");
                display_changed = TRUE;
                detect_state = BL_STOP;
                run_timer = 0;              // [ADDED] Stop the seconds counter
            }
            break;

        case BL_STOP:
            All_Movement_Off();
            state = 0;                      // [FIX] Release back to idle so commands work again
            detect_state = IDLE;            // [FIX] Kill the machine
            follow_state = IDLE;
            break;

        default: break;
    }
}


// void black_line_machine(void){
//     switch (detect_state){
//         case IDLE: break;

//         case PREPARE:
//             // Step 0: Drive forward to find the line
//             P2OUT |= IR_LED;
//             LCD_BACKLITE_DIMING = PERCENT_80;
//             Forward_Med();
//             strcpy(display_line[0], "Intercept ");
//             display_changed = TRUE;
//             detect_state = RUN;
//             break;

//         case RUN:
//             // Step 1: Detect the black line (either sensor)
//             if ((ADC_Left_Detect >= black_value) || (ADC_Right_Detect >= black_value)){
//                 All_Movement_Off();
//                 strcpy(display_line[0], " BL Found ");
//                 display_changed = TRUE;
//                 Timer_Counter = 0;
//                 detect_state = DELAY1;
//             }
//             break;

//         case DELAY1:
//             // Step 2: Small pause after detecting line
//             if (Timer_Counter >= 50){   // 1 second pause (5 * 200ms)
//                 strcpy(display_line[0], " BL Turn  ");
//                 display_changed = TRUE;
//                 Left_Slow_Both();           // Start 90-degree left turn
//                 Timer_Counter = 0;
//                 detect_state = TURN;
//             }
//             break;

//         case TURN:
//             // Step 3: Turn left ~90 degrees (timed turn)
//             Left_Slow_Both();
//             if (Timer_Counter >= 50){  // ~2 seconds turning (10 * 200ms) — tune this!
//                 All_Movement_Off();
//                 Timer_Counter = 0;
//                 detect_state = IDLE;
//                 follow_state = PREPARE; // Hand off to line follower
//             }
//             break;

//         default: break;
//     }
// }
void prepare_case(void){
    P2OUT |= IR_LED;                            // IR LED ON
    All_Movement_Off();                         // Movement OFF
    Forward_Med();                              // Forward MED
    detect_state = WHITE_DETECT;
}

void white_detect_case(void){
    if ((ADC_Left_Detect <= (white_value + 5))
    && (ADC_Left_Detect >= (white_value - 5))){
        All_Movement_Off();
        Timer_Counter = 0;                           // Reset 200ms counter
        detect_state = DELAY1;
        strcpy(display_line[0], " BL Start ");
        display_changed = TRUE;
        LCD_BACKLITE_DIMING = PERCENT_80;
    }
}

void delay_case_1(void){
    if (Timer_Counter >= 50){                        // 10s delay (50 * 200ms)
        detect_state = RUN;
        LCD_BACKLITE_DIMING = WHEEL_OFF;            // LCD light OFF
        Forward_Med();
    }
}

void run_case(void){
    if ((ADC_Left_Detect >= black_value)
    && (ADC_Right_Detect >= black_value)){
        All_Movement_Off();
        strcpy(display_line[0], "Intercept ");
        display_changed = TRUE;
        LCD_BACKLITE_DIMING = PERCENT_80;
        detect_state = DELAY2;
        Timer_Counter = 0;                           // Reset 200ms counter
    }
}

void delay_case_2(void){
    if (Timer_Counter >= 50){                        // 10s delay (50 * 200ms)
        strcpy(display_line[0], " BL Turn  ");
        display_changed = TRUE;
        detect_state = TURN;
        Left_Slow();
    }
}

void turn_case(void){
    if (ADC_Right_Detect >= black_value){
        All_Movement_Off();
        detect_state = STOP;
        Timer_Counter = 0;
    }
}

void stop_case(void){
    if (Timer_Counter >= 50){                        // 10s delay (50 * 200ms)
        detect_state = IDLE;
        follow_state = PREPARE;                     // Hand off to Line Follower
    }
}


// ============================================================================
// 2. LINE FOLLOWING MACHINE
// ============================================================================
void line_follow_machine(void){
    switch (follow_state){
        case IDLE: break;
        case PREPARE:
            follow_prepare_case();
            break;
        case DELAY1:
            follow_delay_case_1();
            break;
        case RUN:
            follow_run_case();
            break;
        case DELAY2:
            follow_delay_case_2();
            break;
        case STOP:
            follow_stop_case();
            break;
        default: break;
    }
}

void follow_prepare_case(void){
    All_Movement_Off();
    strcpy(display_line[0], " BL Travel");
    display_changed = TRUE;
    follow_state = DELAY1;
    follow_or_not = FOLLOW;
    Timer_Counter = 0;
}

void follow_delay_case_1(void){
    if (Timer_Counter >= 25){                        // 10s delay (50 * 200ms)
        LCD_BACKLITE_DIMING = PERCENT_80;;
        follow_state = RUN;
        last_movement = FORWARD;
        Forward_Med();                              // kick start
        Timer_Counter = 0;
    }
}

void follow_run_case(void){
    // Check if we should stop following
    if (follow_or_not == DONT_FOLLOW){
        All_Movement_Off();
        P2OUT &= ~IR_LED;
        strcpy(display_line[0], " BL Exit  ");
        display_changed = TRUE;
        LCD_BACKLITE_DIMING = PERCENT_80;
        follow_state = DELAY2;
        Timer_Counter = 0;
    }
    // Kirk's differential line following logic
    else {
        int ir_diff = (int)ADC_Left_Detect - (int)ADC_Right_Detect;

        // Always clear all motors first for safety
        All_Movement_Off();

        if (abs(ir_diff) > 150) {
            // Sensors see very different values — one on black, one on white
            if (ir_diff > 0) {
                // Left reads higher (blacker) — turn right
                Right_Slow();
            } else {
                // Right reads higher (blacker) — turn left
                Left_Slow();
            }
        }
        else {
            // Both sensors reading similar values
            if ((ADC_Left_Detect >= black_value) || (ADC_Right_Detect >= black_value)) {
                // At least one on black — drive forward
                Forward_Slow_Tweak();
            }
            else {
                // Both on white — lost the line — reverse
                Reverse_Slow2();
            }
        }
    }
}
// void follow_run_case(void){
//     // Finish black line following
//     if (follow_or_not == DONT_FOLLOW){
//         All_Movement_Off();
//         P2OUT &= ~IR_LED;
//         strcpy(display_line[0], " BL Exit  ");
//         display_changed = TRUE;
//         LCD_BACKLITE_DIMING = PERCENT_80;
//         follow_state = DELAY2;
//         Timer_Counter = 0;
//     }
//     // Continue black line following
//     else {
//         // 6s to 16s after following (30 ticks to 80 ticks @ 200ms)
//         if ((Timer_Counter >= 30) && (Timer_Counter <= 80)){
//             if (LCD_BACKLITE_DIMING == PERCENT_80){
//                 strcpy(display_line[0], " BL Circle");
//                 display_changed = TRUE;
//                 LCD_BACKLITE_DIMING = PERCENT_80;
//                 All_Movement_Off();
//             }
//         }
//         else{
//             if (Timer_Counter == 81){                    // Right after 16s
//                 LCD_BACKLITE_DIMING = PERCENT_80;;
//                 Forward_Med();                          // kick start
//                 last_movement = FORWARD;
//             }

//             if ((ADC_Left_Detect < black_value)         // If only left white
//             && !(ADC_Right_Detect < black_value)){
//                 if (last_movement != TURN_RIGHT){
//                     All_Movement_Off();
//                     Right_Slow();
//                     last_movement = TURN_RIGHT;
//                 }
//             }
//             else if (!(ADC_Left_Detect < black_value)   // If only right white
//             && (ADC_Right_Detect < black_value)){
//                 if (last_movement != TURN_LEFT){
//                     All_Movement_Off();
//                     Left_Slow();
//                     last_movement = TURN_LEFT;
//                 }
//             }
//             else if (!(ADC_Left_Detect < black_value)   // If both black
//             && !(ADC_Right_Detect < black_value)){
//                 if (last_movement != FORWARD){
//                     All_Movement_Off();
//                     Forward_Slow();
//                     last_movement = FORWARD;
//                 }
//             }
//             else{                                       // If both white
//                 if ((last_movement == FORWARD)
//                 && (last_movement != TURN_RIGHT)){
//                     All_Movement_Off();
//                     Right_Slow_Reverse();
//                     last_movement = TURN_RIGHT;
//                 }
//             }
//         }
//     }
// }

void follow_delay_case_2(void){
    if (Timer_Counter >= 50){                    // 10s delay (50 * 200ms)
        LCD_BACKLITE_DIMING = WHEEL_OFF;
        follow_state = STOP;
        Forward_Fast();
        Timer_Counter = 0;
    }
}

void follow_stop_case(void){
    if (Timer_Counter >= 13){                    // 2.6s delay (13 * 200ms)
        All_Movement_Off();
        strcpy(display_line[0], " BL Stop  ");
        display_changed = TRUE;
        follow_state = IDLE;
        ready_to_run_command = READY;
        // AHHHHHHHH
        run_timer = 0;
    }
}


















void All_Movement_Off(void) {
    RIGHT_FORWARD_SPEED = WHEEL_OFF;
    LEFT_FORWARD_SPEED  = WHEEL_OFF;
    RIGHT_REVERSE_SPEED = WHEEL_OFF;
    LEFT_REVERSE_SPEED  = WHEEL_OFF;
}

void Forward_Fast(void) {
    All_Movement_Off(); // Always stop first to prevent H-bridge shoot-through
    RIGHT_FORWARD_SPEED = FAST2;
    LEFT_FORWARD_SPEED  = FAST;
}

void Forward_Slow(void) {
    All_Movement_Off();
    RIGHT_FORWARD_SPEED = SLOW;
    LEFT_FORWARD_SPEED  = SLOW;
}

void Forward_Slow_Tweak(void){
    Wheels_Stop();
    RIGHT_FORWARD_SPEED = MID;
    LEFT_FORWARD_SPEED = SLOW;
}

// For a right turn: left wheel drives forward, right wheel stops
void Right_Slow(void) {
    All_Movement_Off();
    RIGHT_FORWARD_SPEED = WHEEL_OFF;
    LEFT_FORWARD_SPEED  = SLOW;
}

void Right_Slow3_Both(void){
    All_Movement_Off();
    RIGHT_REVERSE_SPEED = SLOW3;
    LEFT_FORWARD_SPEED = SLOW3;
}

void Left_Slow_Reverse(void) {
    All_Movement_Off();
    RIGHT_FORWARD_SPEED = WHEEL_OFF; 
    LEFT_REVERSE_SPEED = SLOW;
}

void Right_Slow_Reverse (void){
    All_Movement_Off();
    RIGHT_REVERSE_SPEED = SLOW;
    LEFT_FORWARD_SPEED = WHEEL_OFF;
}

void Right_Slow_Both (void){
    All_Movement_Off();
    RIGHT_REVERSE_SPEED = SLOW;
    LEFT_FORWARD_SPEED = SLOW;
}



void Left_Slow_Both (void){
    All_Movement_Off();
    RIGHT_FORWARD_SPEED = SLOW;
    LEFT_REVERSE_SPEED = SLOW;
}

// For a left turn: right wheel drives forward, left wheel stops
void Left_Slow(void) {
    All_Movement_Off();
    RIGHT_FORWARD_SPEED = SLOW;
    LEFT_FORWARD_SPEED  = WHEEL_OFF;
}

// Used to kick-start the car during the PREPARE and DELAY states
void Forward_Med(void) {
    All_Movement_Off();
    // I am using 25000 here as a midpoint. Feel free to add
    // #define MED (25000) to your macros.h if you want it standardized!
    RIGHT_FORWARD_SPEED = 16500;
    LEFT_FORWARD_SPEED  = 16500;
}


void Reverse_Slow(void) {
    All_Movement_Off();
    RIGHT_REVERSE_SPEED = SLOW;
    LEFT_REVERSE_SPEED  = SLOW;
}

void Reverse_Slow2(void){
    All_Movement_Off();
    RIGHT_REVERSE_SPEED = SLOW2;
    LEFT_REVERSE_SPEED  = SLOW2;
}

// ============================================================================
// [ADDED] Project 10 — Arc Turn Functions
// Both motors forward at different speeds for a curved path
// arc_diff controls the radius — set by command value
// ============================================================================
void Arc_Right(unsigned int right_spd, unsigned int left_spd) {
    // [ADDED] Arc right: left wheel faster, right wheel slower
    All_Movement_Off();
    RIGHT_FORWARD_SPEED = right_spd;
    LEFT_FORWARD_SPEED  = left_spd;
}

void Arc_Left(unsigned int right_spd, unsigned int left_spd) {
    // [ADDED] Arc left: right wheel faster, left wheel slower
    All_Movement_Off();
    RIGHT_FORWARD_SPEED = right_spd;
    LEFT_FORWARD_SPEED  = left_spd;
}