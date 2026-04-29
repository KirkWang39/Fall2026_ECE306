/*
 * switches.c
 *
 *  Created on: Feb 19, 2026
 *      Author: Justin Banh
 */
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

extern volatile unsigned int Timer_Counter;      // From timer.c (ensure it ticks every 20ms!)
extern unsigned char detect_state;       // Starts idle
extern char run_timer;
extern unsigned int project_time;

/////// TIMER FOR HW7
extern volatile unsigned int transmit_delay_counter;
extern volatile char start_transmission;

extern volatile unsigned char update_display;


extern unsigned int proj6_substate;

extern char display_line[4][11];
extern volatile unsigned char display_changed;
extern unsigned int state; // Your movement state variable

// Global flags for the ISRs to communicate with the main loop
volatile unsigned char sw1_pressed;
volatile unsigned char sw2_pressed;


////// SOMEBODY SAVE ME
extern volatile unsigned int ADC_Left_Detect;
extern volatile unsigned int ADC_Right_Detect;
extern unsigned int black_value;
////////// SET MY MOTORS

// Pulling from serial.c
extern char command_buffer[11];
extern volatile unsigned int tx_index;
extern volatile unsigned int cmd_index;
unsigned int current_baud_state = 0; // 0: 115200, 1: 57600, 2: 460800

///project8 global
volatile unsigned char speed;
extern volatile unsigned int iot_reset_counter;
extern volatile char iot_reset_active;

//test motor timer
extern volatile unsigned int wheel_test_timer;
extern volatile unsigned int iot_movement_timer; // Links to timer.c
extern volatile unsigned int iot_startup_timer;
extern volatile unsigned int iot_startup_state;

#pragma vector=PORT4_VECTOR
__interrupt void switch1_interrupt(void) {
    if (P4IFG & SW1) {
        P4IE &= ~SW1;                      // Disable SW1 interrupt
        P4IFG &= ~SW1;                     // Clear SW1 flag
        TB0CCTL1 &= ~CCIFG;
        TB0CCR1  = TB0R + TB0CCR1_INTERVAL;// Start debounce timer
        TB0CCTL1 |= CCIE;

        sw1_pressed = TRUE;
    }
}

#pragma vector=PORT2_VECTOR
__interrupt void switch2_interrupt(void) {
    if (P2IFG & SW2) {
        P2IE &= ~SW2;                      // Disable SW2 interrupt
        P2IFG &= ~SW2;                     // Clear SW2 flag
        TB0CCTL2 &= ~CCIFG;
        TB0CCR2 = TB0R + TB0CCR2_INTERVAL; // Start debounce timer
        TB0CCTL2 |= CCIE;

        sw2_pressed = TRUE;
    }
}
//-----------------------------------------------------------------------------
// Switch Processing (Safe to do heavy lifting here)
//-----------------------------------------------------------------------------



void Switches_Process(void) {
    Switch1_Process();
    Switch2_Process();
}

void Switch1_Process(void) {
    if (sw1_pressed) {
        sw1_pressed = FALSE;   // Acknowledge the press


    //project9 logics:

    // // --- PHASE 4: START IOT HARDWARE RESET ---
    //     P3OUT &= ~IOT_RN_CPU;      // Pull IOT_EN low
    //     iot_reset_counter = 5;     // 5 ticks * 20ms = 100ms
    //     iot_reset_active = 1;      // Tell the timer to start counting down  

    //     // Update LCD to show reset status
    //     strcpy(display_line[0], " IOT RESET");
    //     strcpy(display_line[1], "          ");
    //     strcpy(display_line[2], "          ");
    //     strcpy(display_line[3], "          "); 

    //     display_changed = TRUE;
    //     update_display = TRUE;

// 1. Update LCD
        strcpy(display_line[0], "  SERVER  ");
        strcpy(display_line[1], " STARTING ");
        strcpy(display_line[2], "          ");
        strcpy(display_line[3], "          "); 
        display_changed = TRUE;
        update_display = TRUE;

// 2. Set the state and fire the first command
        iot_startup_state = 1;
        Send_IoT_Command("AT+CIFSR\r\n");

    
//         // project 8 Logic
// // Move the received command from Line 4 to Line 2
//         strcpy(display_line[1], display_line[3]);
        
//         // Update statuses
//         strcpy(display_line[0], " Transmit ");
//         strcpy(display_line[3], "          "); // Clear bottom line

//         display_changed = TRUE;
//         update_display = TRUE;

//         // Reset the TX tracker and fire the transmit interrupt
//         tx_index = 0;
//         UCA0IE |= UCTXIE;
    }
}

void Switch2_Process(void) {
    if (sw2_pressed) {
        sw2_pressed = FALSE;   // Acknowledge the press


        strcpy(display_line[0], " Waiting  ");
        strcpy(display_line[1], "for input ");
        strcpy(display_line[2], "          ");
        strcpy(display_line[3], "          ");
        display_changed = TRUE;
        Display_Update(0, 0, 0, 0);

        // // [ADDED] Project 10 — Quick test: same as sending ^7891I via IoT
        // P2OUT |= IR_LED;
        // detect_state = BL_START_DELAY;
        // state = 1;
        // run_timer = 1;
        // project_time = 0;
        // Timer_Counter = 0;
        // strcpy(display_line[0], " BL Start ");
        // display_changed = TRUE;

        

// //project8 
// // Cycle through the baud rates (0 -> 1 -> 2 -> 0)
//         current_baud_state++;
//         if (current_baud_state > 2) {
//             current_baud_state = 0;
//         }

//         // Reset the display to the "Waiting" State
//         strcpy(display_line[0], "Waiting   ");
//         strcpy(display_line[1], "          ");
//         strcpy(display_line[3], "          ");
//         cmd_index = 0; // Reset character tracking for the new speed

//         if (current_baud_state == 0) {
//             Init_Serial_UCA0(BAUD_115200);
//             Init_Serial_UCA1(BAUD_115200);
//             strcpy(display_line[2], "  115200  ");
//         } else if (current_baud_state == 1) {
//             Init_Serial_UCA0(BAUD_57600);
//             Init_Serial_UCA1(BAUD_57600);
//             strcpy(display_line[2], "  57600   ");
//         } else if (current_baud_state == 2) {
//             Init_Serial_UCA0(BAUD_460800);
//             Init_Serial_UCA1(BAUD_460800);
//             strcpy(display_line[2], "  460800  ");
//         }

//         display_changed = TRUE;
//         update_display = TRUE;
// // HW 8
//         Init_Serial_UCA0(BAUD_460800);
//                 Init_Serial_UCA1(BAUD_460800);

//                 // 2. Clear Lines 1 & 2, Update Line 4
//                 strcpy(display_line[0], "          ");
//                 strcpy(display_line[1], "          ");
//                 strcpy(display_line[2], "   Baud   ");
//                 strcpy(display_line[3], " 460,800  ");

//                 display_changed = TRUE;
//                 update_display = TRUE;
//                 display_index = 0; // Reset LCD printing cursor to the left

//                 // 3. Start 2-Second Delay (10 ticks of 200ms)
//                 transmit_delay_counter = 10;
//                 start_transmission = 1;
    }
}
