/*
 * serial.c
 *
 *  Created on: Mar 26, 2026
 *      Author: K
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



// Global Variables for Rings and Flow
volatile char IOT_Ring_Rx[RING_BUF_SIZE];
volatile char USB_Ring_Rx[RING_BUF_SIZE];
volatile char iot_TX_buf[RING_BUF_SIZE];

volatile unsigned int iot_rx_wr = 0;
volatile unsigned int usb_rx_wr = 0;
volatile unsigned int iot_tx = 0;
volatile unsigned int direct_iot = 0;
volatile unsigned int display_index = 0;

// HW8 2-Second Delay Variables
volatile unsigned int transmit_delay_counter = 0;
volatile char start_transmission = 0;

// External display variables from your existing code
extern char display_line[4][11];
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display; // Forces the screen to refresh!

// Project 8 Serial Variables
char command_buffer[11] = "          ";
volatile unsigned int cmd_index = 0;
volatile unsigned int tx_index = 0;



//project 9 Vals
volatile char pc_rx_ready = 0;
volatile char fram_cmd_active = 0; 
volatile char fram_cmd_char = 0;

extern unsigned int ring_index;
extern char ip_address[16];
extern unsigned int ip_index;
extern char capturing_ip;
extern unsigned int parse_state; // State machine tracker
// --- COMMAND PARSER VARIABLES ---
unsigned int cmd_parse_state = 0;
char cmd_direction = 0;
unsigned int cmd_time_val = 0;
extern volatile unsigned int iot_movement_timer; // Links to timer.c

extern unsigned int time_digits; 
// send iot command var
volatile unsigned int iot_tx_index = 0;
volatile unsigned int iot_tx_len = 0;

extern volatile unsigned int iot_startup_timer;
extern volatile unsigned int iot_startup_state;
extern unsigned int ok_parse_state;

extern volatile unsigned int ADC_Channel;      // Tracks which channel is being read
extern volatile unsigned int ADC_Left_Detect;  // Stores Left IR Detector value
extern volatile unsigned int ADC_Right_Detect; // Stores Right IR Detector value
extern volatile unsigned int ADC_Thumb;        // Stores Thumbwheel value

extern unsigned int project_time; // Tracks total ticks
extern char run_timer;            // Flag to turn timer on/off

extern Timer_Counter;


// ============================================================================
// [ADDED] Project 10 — Globals for the full autonomous sequence
// ============================================================================
extern unsigned int bl_timer;                // Timer for black line delays (incremented by Timer_Counter)
extern unsigned int bl_timer_last;           // [ADDED] Tracks last Timer_Counter to detect ticks
extern unsigned int circle_timer;            // Timer to detect when car enters circle
extern unsigned int exit_drive_time;       // [ADDED] How long to drive on exit (default 5s = 250*20ms)
extern unsigned int arc_right_speed;      // [ADDED] Arc right motor speed
extern unsigned int arc_left_speed;       // [ADDED] Arc left motor speed
extern unsigned int arc_diff;             // [ADDED] Speed difference for arc (cmd_time_val sets this)




// void parse_ip(void) {
//     // --- PHASE 5: PARSE THE IP ADDRESS ---
//     while (ring_index != iot_rx_wr) {
//         char c = IOT_Ring_Rx[ring_index];

//         // State Machine to specifically find 'I' -> 'P' -> ',' -> '"'
//         if (parse_state == 0 && c == 'I') { parse_state = 1; }
//         else if (parse_state == 1 && c == 'P') { parse_state = 2; }
//         else if (parse_state == 2 && c == ',') { parse_state = 3; }
//         else if (parse_state == 3 && c == '"') {
//             parse_state = 4; // We found the exact start of the IP!
//             ip_index = 0;
//         }
//         else if (parse_state == 4) {
//             // We are actively capturing the IP address
//             if (c == '"') {
//                 // We hit the closing quote. Done capturing!
//                 ip_address[ip_index] = '\0'; // Null terminate it
//                 parse_state = 0;             // Reset the state machine

//                 // Display it on the LCD
//                 strcpy(display_line[0], "   ncsu   ");
//                 strcpy(display_line[1], "IP address");
                
//                 strncpy(display_line[2], ip_address, 10);
//                 strncpy(display_line[3], ip_address + 10, 10);
                
//                 display_changed = TRUE;
//                 update_display = TRUE;
//             } else if (ip_index < 15) {
//                 ip_address[ip_index++] = c; // Store the number
//             }
//         }
//         else {
//             // If the sequence breaks, reset back to searching
//             if (parse_state != 4) {
//                 parse_state = 0;
//             }
//         }

//         ring_index++;
//         if (ring_index >= sizeof(IOT_Ring_Rx)) {
//             ring_index = BEGINNING;
//         }
//     }
// }

// void process_iot_buffer(void) {
//     // CRITICAL: If the car is moving, STOP parsing! 
//     // This leaves the next command safely waiting in the ring buffer for chained commands.
//     if (iot_movement_timer > 0) { return; }

//     while (ring_index != iot_rx_wr) {
//         char c = IOT_Ring_Rx[ring_index];

//         // ----------------------------------------------------
//         // 1. IP ADDRESS PARSER (From Phase 5)
//         // ----------------------------------------------------
//         if (parse_state == 0 && c == 'I') { parse_state = 1; }
//         else if (parse_state == 1 && c == 'P') { parse_state = 2; }
//         else if (parse_state == 2 && c == ',') { parse_state = 3; }
//         else if (parse_state == 3 && c == '"') { parse_state = 4; ip_index = 0; }
//         else if (parse_state == 4) {
//             if (c == '"') {
//                 ip_address[ip_index] = '\0';
//                 parse_state = 0;
//                 strcpy(display_line[0], "   ncsu   ");
//                 strcpy(display_line[1], "IP address");
//                 strncpy(display_line[2], ip_address, 10);
//                 strncpy(display_line[3], ip_address + 10, 10);
//                 display_changed = TRUE;
//                 update_display = TRUE;
//             } else if (ip_index < 15) {
//                 ip_address[ip_index++] = c;
//             }
//         } else { if (parse_state != 4) { parse_state = 0; } }

//         // ----------------------------------------------------
//         // 2. MOVEMENT COMMAND PARSER (Looks for ^1234F0100)
//         // ----------------------------------------------------
//         if (cmd_parse_state == 0 && c == '^') { cmd_parse_state = 1; }
//         else if (cmd_parse_state == 1 && c == '7') { cmd_parse_state = 2; } // PIN: 1
//         else if (cmd_parse_state == 2 && c == '8') { cmd_parse_state = 3; } // PIN: 2
//         else if (cmd_parse_state == 3 && c == '9') { cmd_parse_state = 4; } // PIN: 3
//         else if (cmd_parse_state == 4 && c == '1') { cmd_parse_state = 5; } // PIN: 4
//         else if (cmd_parse_state == 5) {
//             // Check for valid direction
//             if (c == 'F' || c == 'B' || c == 'R' || c == 'L') {
//                 cmd_direction = c;
//                 cmd_parse_state = 6;
//                 cmd_time_val = 0;
//             } else {
//                 cmd_parse_state = 0; // Invalid command, reset
//             }
//         }
//         else if (cmd_parse_state == 6) {
//             if (c >= '0' && c <= '9') {
//                 cmd_time_val = (cmd_time_val * 10) + (c - '0'); // Build the integer
//             } 
//             else if (c == '\r' || c == '\n') {
//                 // COMMAND IS FULLY BUILT! EXECUTE IT!
//                 cmd_parse_state = 0;
                
//                 // Display the command on the LCD
//                 strcpy(display_line[1], " COMMAND: ");
//                 display_line[2][0] = ' '; display_line[2][1] = ' '; display_line[2][2] = ' ';
//                 display_line[2][3] = '^'; display_line[2][4] = cmd_direction;
//                 display_line[2][5] = ' '; display_line[2][6] = ' '; display_line[2][7] = ' ';
//                 display_line[2][8] = ' '; display_line[2][9] = ' '; display_line[2][10] = '\0';
//                 display_changed = TRUE;
//                 update_display = TRUE;

//                 // Fire the motors using your wheels.c functions
//                 if (cmd_direction == 'F') { Wheels_Forward(); }
//                 else if (cmd_direction == 'B') { Wheels_Reverse(); }
//                 else if (cmd_direction == 'R') { Wheels_Spin_CW(); }
//                 else if (cmd_direction == 'L') { Wheels_Spin_CCW(); }

//                 iot_movement_timer = cmd_time_val; // Start the countdown!

//                 // IMPORTANT: Break out of the parsing loop so we don't accidentally 
//                 // process a chained command until this one finishes moving!
//                 ring_index++; 
//                 if (ring_index >= RING_BUF_SIZE) { ring_index = 0; }
//                 break; 
//             }
//         }
//         else { if (cmd_parse_state != 6) { cmd_parse_state = 0; } }

//         ring_index++;
//         if (ring_index >= RING_BUF_SIZE) { ring_index = 0; } // Assuming RING_BUF_SIZE is defined
//     }
// }

void process_iot_buffer(void) {
    // If the car is moving, STOP parsing! 
    if (iot_movement_timer > 0) { return; }

    while (ring_index != iot_rx_wr) {
        char c = IOT_Ring_Rx[ring_index];

        // ----------------------------------------------------
        // 1. IP ADDRESS PARSER 
        // ----------------------------------------------------
        if (parse_state == 0 && c == 'I') { parse_state = 1; }
        else if (parse_state == 1 && c == 'P') { parse_state = 2; }
        else if (parse_state == 2 && c == ',') { parse_state = 3; }
        else if (parse_state == 3 && c == '"') { parse_state = 4; ip_index = 0; }
        else if (parse_state == 4) {
            if (c == '"') {
                ip_address[ip_index] = '\0';
                parse_state = 0;
                strcpy(display_line[0], "   ncsu   ");
                strcpy(display_line[1], "IP address");
                strncpy(display_line[2], ip_address, 10);
                strncpy(display_line[3], ip_address + 10, 10);
                display_changed = TRUE;
                update_display = TRUE;
            } else if (ip_index < 15) {
                ip_address[ip_index++] = c;
            }
        } else { if (parse_state != 4) { parse_state = 0; } }

        // ----------------------------------------------------
        // 2. MOVEMENT COMMAND PARSER (Using PIN 7891)
        // ----------------------------------------------------
        if (cmd_parse_state == 0 && c == '^') { cmd_parse_state = 1; }
        else if (cmd_parse_state == 1 && c == '7') { cmd_parse_state = 2; } 
        else if (cmd_parse_state == 2 && c == '8') { cmd_parse_state = 3; } 
        else if (cmd_parse_state == 3 && c == '9') { cmd_parse_state = 4; } 
        else if (cmd_parse_state == 4 && c == '1') { cmd_parse_state = 5; } 
        else if (cmd_parse_state == 5) {
            if (c == 'F' || c == 'B' || c == 'R' || c == 'L'
             || c == 'W' || c == 'K'                          // [ADDED] Calibration
             || c == 'I'                                      // [ADDED] Intercept (autonomous)
             || c == 'E'                                      // [ADDED] Exit circle
             || c == 'A' || c == 'D'                          // [ADDED] Arc turns
             || c == 'S'
             || c == 'X'
             || c == 'G') {                                   // [ADDED] Arrived display
                cmd_direction = c;
                cmd_parse_state = 6;
                cmd_time_val = 0;
            } else {
                cmd_parse_state = 0; 
            }
        }
        else if (cmd_parse_state == 6) {
            if (c >= '0' && c <= '9') {
                cmd_time_val = (cmd_time_val * 10) + (c - '0'); 
            } 
            // NEW CHAINING LOGIC: Trigger on a Return OR a new '^'
            else if (c == '\r' || c == '\n' || c == '^') {
                cmd_parse_state = 0;
                
                // [MODIFIED] Project 10 — Display format per spec------------------------------------------------------
                // Line 0: "Arrived 0X" is set by S command or BL states
                // Lines 1-2: Keep IP address / Name (don't overwrite here)
                // Line 3: Show last command + seconds (left side only)
                display_line[3][0] = cmd_direction;
                // Convert cmd_time_val to 4 digits for display
                display_line[3][1] = '0' + ((cmd_time_val / 1000) % 10);
                display_line[3][2] = '0' + ((cmd_time_val / 100) % 10);
                display_line[3][3] = '0' + ((cmd_time_val / 10) % 10);
                display_line[3][4] = '0' + (cmd_time_val % 10);
                // Positions 5-9 are updated by the seconds counter in main.c
                display_changed = TRUE;
                update_display = TRUE;

                // [ADDED] Start the seconds counter on first command received
                if (run_timer == 0) {
                    run_timer = 1;
                    project_time = 0;
                }
                
                // else if (cmd_direction == 'K') {
                //     P2OUT |= IR_LED;
                //     black_value = (ADC_Left_Detect + ADC_Right_Detect) / 2 -30;
                //     // [MODIFIED] Show "Waiting for input" after calibration complete
                //     strcpy(display_line[0], " Waiting  ");
                //     strcpy(display_line[1], "for input ");
                //     HEXtoBCD(black_value);
                //     adc_line(3, 3);
                //     display_changed = TRUE;
                // }
                // ==============================================================
                // [ADDED] Project 10 — Calibration Commands (no motors, no timer)
                // ==============================================================
                if (cmd_direction == 'W') {
                    P2OUT |= IR_LED;
                    white_value = (ADC_Left_Detect + ADC_Right_Detect) / 2 +30; //---------------------------------------------------------------
                    strcpy(display_line[0], " WhiteCal ");
                    HEXtoBCD(white_value);
                    adc_line(2, 3);
                    display_changed = TRUE;
                }
                else if (cmd_direction == 'K') {
                    P2OUT |= IR_LED;
                    black_value = (ADC_Left_Detect + ADC_Right_Detect) / 2 -30;   //-------------------------------------------------------------------
                    strcpy(display_line[0], " BlackCal ");
                    HEXtoBCD(black_value);
                    adc_line(3, 3);
                    display_changed = TRUE;
                }
                // ==============================================================
                // [ADDED] Project 10 — Autonomous Intercept Command
                // Send: ^7891I
                // ==============================================================
                else if (cmd_direction == 'I') {
                    P2OUT |= IR_LED;
                    detect_state = BL_START_DELAY;
                    follow_state = IDLE;
                    state = 1;
                    run_timer = 1;
                    project_time = 0;
                    Timer_Counter = 0;
                }
                // ==============================================================
                // [ADDED] Project 10 — Exit Circle Command
                // Send: ^7891E0250  (0250 = drive time in 20ms ticks)
                // ==============================================================
                else if (cmd_direction == 'E') {
                    if (cmd_time_val > 0) {
                        exit_drive_time = cmd_time_val;
                    }
                    detect_state = BL_EXIT;
                    Timer_Counter = 0;      // [FIX] Reset so exit delay works
                }
                // ==============================================================
                // [ADDED] Project 10 — Arc Right Turn (curved path)
                // Send: ^7891A5000  (5000 = speed difference between wheels)
                // The number sets arc_diff: bigger = tighter turn
                // Uses iot_movement_timer from a separate chained F command for timing
                // ==============================================================
                else if (cmd_direction == 'A') {
                    if (cmd_time_val > 0) {
                        arc_diff = cmd_time_val;        // [ADDED] Set the speed difference
                    }
                    // [ADDED] Arc right: left=SLOW, right=SLOW-diff (right is inner wheel)
                    Arc_Right(SLOW - arc_diff, SLOW);
                    // [ADDED] Display "Arrived 08" during arc toward pad 8
                    // strcpy(display_line[0], "Arrived 08");
                    // display_changed = TRUE;
                    iot_movement_timer = 400;           // [ADDED] Default 8s arc, override with chain
                }
                // ==============================================================
                // [ADDED] Project 10 — Arc Left Turn
                // Send: ^7891D5000
                // ==============================================================
                else if (cmd_direction == 'D') {
                    if (cmd_time_val > 0) {
                        arc_diff = cmd_time_val;
                    }
                    // [ADDED] Arc left: right=SLOW, left=SLOW-diff (left is inner wheel)
                    Arc_Left(SLOW, SLOW - arc_diff);
                    // strcpy(display_line[0], "Arrived 08");
                    // display_changed = TRUE;
                    iot_movement_timer = 400;
                }
                // ==============================================================
                // [ADDED] Project 10 — Arrived Display
                // Send: ^7891S0001 through ^7891S0008
                // ==============================================================
                else if (cmd_direction == 'S') {
                    display_line[0][0] = 'A';
                    display_line[0][1] = 'r';
                    display_line[0][2] = 'r';
                    display_line[0][3] = 'i';
                    display_line[0][4] = 'v';
                    display_line[0][5] = 'e';
                    display_line[0][6] = 'd';
                    display_line[0][7] = ' ';
                    display_line[0][8] = '0';
                    display_line[0][9] = '0' + (cmd_time_val % 10);
                    display_line[0][10] = '\0';
                    display_changed = TRUE;
                }
                // ==============================================================
                // [ADDED] Project 10 — Emergency Stop Command
                // Send: ^7891X
                // Stops all motors immediately, kills BL machine, returns to idle
                // Does NOT drive forward like 'E' does
                // ==============================================================
                else if (cmd_direction == 'X') {
                    All_Movement_Off();
                    iot_movement_timer = 0;
                    detect_state = IDLE;
                    follow_state = IDLE;
                    follow_or_not = DONT_FOLLOW;
                    state = 0;
                    strcpy(display_line[0], "  STOPPED ");
                    display_changed = TRUE;
                }
                // ==============================================================
                // Original movement commands (unchanged)
                // ==============================================================
                else {
                    if (cmd_direction == 'F') { Forward_Slow_Tweak(); }
                    else if (cmd_direction == 'G') { Forward_Fast();   }
                    else if (cmd_direction == 'B') { Wheels_Reverse(); }
                    else if (cmd_direction == 'R') { Wheels_Spin_CW(); }
                    else if (cmd_direction == 'L') { Wheels_Spin_CCW(); }

                    iot_movement_timer = cmd_time_val;
                }

                // THE CHAINING MAGIC
                if (c == '^') {
                    break;
                } else {
                    ring_index++;
                    if (ring_index >= RING_BUF_SIZE) { ring_index = 0; }
                    break;
                }            }
            else { cmd_parse_state = 0; } // Invalid character
        }
        else { if (cmd_parse_state != 6) { cmd_parse_state = 0; } }

        // ----------------------------------------------------
        // 3. "OK" RESPONSE PARSER (For Automated Startup)
        // ----------------------------------------------------
        if (ok_parse_state == 0 && c == 'O') { ok_parse_state = 1; }
        else if (ok_parse_state == 1 && c == 'K') {
            ok_parse_state = 0; // Reset the parser
            
            // We got an OK! Check what command needs to be sent next.
            if (iot_startup_state == 1) {
                // CIFSR finished, now send MUX
                Send_IoT_Command("AT+CIPMUX=1\r\n");
                iot_startup_state = 2;
            } 
            else if (iot_startup_state == 2) {
                // MUX finished, now start Server
                Send_IoT_Command("AT+CIPSERVER=1,7891\r\n");
                iot_startup_state = 3;
            }
            else if (iot_startup_state == 3) {
                // Server started! The sequence is complete.
                iot_startup_state = 0; 
            }
        }
        else { 
            // If the sequence breaks, reset back to searching for 'O'
            if (c != 'O') { ok_parse_state = 0; } 
        }

        // Standard loop increment
        ring_index++;
        if (ring_index >= RING_BUF_SIZE) { ring_index = 0; } 
    }
}

void Send_IoT_Command(char* cmd) {
    strcpy((char*)iot_TX_buf, cmd);
    iot_tx_index = 0;
    iot_tx_len = strlen(cmd);
    UCA0IE |= UCTXIE; // Turn on TX interrupt to fire the commands!
}


//===========================================================================
// Initialization eUSCI_A0 (IOT / Loopback)
//===========================================================================
void Init_Serial_UCA0(char speed){
    UCA0CTLW0 = 0;
    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 |= UCSSEL__SMCLK;
    UCA0CTLW0 &= ~UCMSB;
    UCA0CTLW0 &= ~UCSPB;
    UCA0CTLW0 &= ~UCPEN;
    UCA0CTLW0 &= ~UCSYNC;
    UCA0CTLW0 &= ~UC7BIT;
    UCA0CTLW0 |= UCMODE_0;

    if(speed == BAUD_115200){
            UCA0BRW   = 4;
            UCA0MCTLW = 0x5551;
        } else if(speed == BAUD_460800){
            UCA0BRW   = 1;
            UCA0MCTLW = 0x4A11;
        } else if(speed == BAUD_57600){
            UCA0BRW   = 8;             // From Slide 4 calculations
            UCA0MCTLW = 0xF7A1;        // Combines 0xF7, 0xA, and 1
        }

    UCA0CTLW0 &= ~UCSWRST;
    UCA0TXBUF  = 0x00;        // Prime the pump
    UCA0IE    |= UCRXIE;      // Enable RX interrupt
}

//===========================================================================
// Initialization eUSCI_A1 (PC / Termite)
//===========================================================================
void Init_Serial_UCA1(char speed){
    UCA1CTLW0 = 0;
    UCA1CTLW0 |= UCSWRST;
    UCA1CTLW0 |= UCSSEL__SMCLK;
    UCA1CTLW0 &= ~UCMSB;
    UCA1CTLW0 &= ~UCSPB;
    UCA1CTLW0 &= ~UCPEN;
    UCA1CTLW0 &= ~UCSYNC;
    UCA1CTLW0 &= ~UC7BIT;
    UCA1CTLW0 |= UCMODE_0;

    if(speed == BAUD_115200){
        UCA1BRW   = 4;
        UCA1MCTLW = 0x5551;
    } else if(speed == BAUD_460800){
        UCA1BRW   = 1;
        UCA1MCTLW = 0x4A11;
    }

    UCA1CTLW0 &= ~UCSWRST;
    UCA1TXBUF  = 0x00;        // Prime the pump
    UCA1IE    |= UCRXIE;      // Enable RX interrupt
}

//===========================================================================
// eUSCI_A0 ISR (Receives from IOT Jumper, Sends to PC)
//===========================================================================
#pragma vector = EUSCI_A0_VECTOR
__interrupt void eUSCI_A0_ISR(void) {
    char iot_receive;
    switch (__even_in_range(UCA0IV, 0x08)) {
    case 0: break;
    case 2: // RX from IOT Jumper

//project9
      iot_receive = UCA0RXBUF;

// Save it to the ring buffer for main.c to parse
      IOT_Ring_Rx[iot_rx_wr++] = iot_receive;
      if(iot_rx_wr >= sizeof(IOT_Ring_Rx)){
          iot_rx_wr = BEGINNING; 
      }

      
      if (pc_rx_ready) {
        UCA1TXBUF = iot_receive; // Send directly to Termite
      }
      // (You will add network parsing logic +IPD here later in Phase 6)
      break;

      // iot_receive = UCA0RXBUF;
      // // Ignore carriage returns/line feeds
      // if ((iot_receive != '\n') && (iot_receive != '\r') ){
      //     if (cmd_index == 0){
      //         strcpy(display_line[0], "Received  ");
      //         strcpy(display_line[3], "          "); // Clear previous cmd
      //     }
      //     display_line[3][cmd_index] = iot_receive;
      //     command_buffer[cmd_index] = iot_receive;
      //     cmd_index++;

      //     // Lock it in after 10 characters
      //     if (cmd_index >= 10) {
      //         cmd_index = 0;
      //     }

      //     display_changed = TRUE;
      //     update_display = TRUE;

      // }

      // break;
        case 4: // TX
        // --- UPGRADED TX INTERRUPT ---
        UCA0TXBUF = iot_TX_buf[iot_tx_index++]; // Send current character
        
        if(iot_tx_index >= iot_tx_len){
            UCA0IE &= ~UCTXIE; // Turn off TX interrupt when full string is sent
        }
        break;
    default: break;
    }
}

//===========================================================================
// eUSCI_A1 ISR (Receives from PC, Sends to IOT, Prints to LCD)
//===========================================================================
#pragma vector = EUSCI_A1_VECTOR
__interrupt void eUSCI_A1_ISR(void) {
    char usb_value;
    switch (__even_in_range(UCA1IV, 0x08)) {
    case 0: break;
    case 2: // RX from PC (Termite)

//project9
      usb_value = UCA1RXBUF;
      pc_rx_ready = 1;

      if (usb_value == '^') {
        fram_cmd_active = 1; // Start of FRAM command
      } else if (fram_cmd_active) {
        if (usb_value == '\r' || usb_value == '\n') {
          // Execute command
          if (fram_cmd_char == 'F') {
            Init_Serial_UCA0(BAUD_115200);
          }
          if (fram_cmd_char == 'S') {
            Init_Serial_UCA0(BAUD_57600); /* or 9600 based on macro */
          }
          fram_cmd_active = 0;
        } else {
          fram_cmd_char = usb_value; // Store F or S
        }
      } else {
        // Not a FRAM command, pass through to IoT module
        UCA0TXBUF = usb_value;
      }
      break;
      // usb_value = UCA1RXBUF;
      // USB_Ring_Rx[usb_rx_wr++] = usb_value;
      // if(usb_rx_wr >= sizeof(USB_Ring_Rx)){
      //     usb_rx_wr = BEGINNING;
      // }

      // UCA0TXBUF = usb_value; // Echo to UCA0 (Hardware Loopback)

      // // --- LCD PRINTING LOGIC ---
      // display_line[0][display_index++] = usb_value;
      // if(display_index >= 10){
      //     display_index = 0; // Wrap around if we hit the edge of the screen
      // }
      // display_changed = TRUE;
      // update_display = TRUE; // CRITICAL: Tell main.c to draw immediately!
      // break;

    case 4: // TX
        UCA1TXBUF = IOT_Ring_Rx[direct_iot++];
        if(direct_iot >= sizeof(IOT_Ring_Rx)){
            direct_iot = BEGINNING;
        }
        if(iot_rx_wr == direct_iot){
            UCA1IE &= ~UCTXIE; // Turn off TX interrupt when done
        }
        break;
    default: break;
    }
}
