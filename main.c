//------------------------------------------------------------------------------
//
//  Description: This file contains the Main Routine - "While" Operating System
//
//  Jim Carlson
//  Jan 2023
//  Built with Code Composer Version: CCS12.4.0.00007_win64
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

/*
 =============================================================================
 ESP32 IOT MODULE - AT COMMAND REFERENCE GUIDE
 =============================================================================
 NOTE: You MUST configure Termite to append CR-LF (Carriage Return + Line Feed)
       to all commands, or the module will ignore you.

AT+RESTORE  dont do this, only when reset everything
AT+SYSSTORE=1
AT+CWMODE=1
AT+CIPSTAMAC="14:33:5C:67:69:68"

AT+CWJAP="ncsu",""
AT+CWHOSTNAME="Car7891"
AT+CWAUTOCONN=1
AT+CWRECONNCFG=5,100
AT+SYSSTORE=0

AT+CIFSR
AT+CIPMUX=1
AT+CIPSERVER=1,7891

 =============================================================================
*/
#include "LCD.h"
#include "adc.h"
#include "cntlpwm.h"
#include "dac.h"
#include "functions.h"
#include "macros.h"
#include "msp430.h"
#include "ports.h"
#include "serial.h"
#include <string.h>
#include <timer.h>


// Function Prototypes
void main(void);
void Init_Conditions(void);
void Display_Process(void);
void Init_LEDs(void);
void Carlson_StateMachine(void);

// Global Variables
volatile char slow_input_down;
char display_line[4][11];
char *display[4];
unsigned char display_mode;
volatile unsigned char display_changed;
extern volatile unsigned int update_display_count;
extern volatile unsigned char update_display; // Forces the screen to refresh!
extern volatile unsigned int Time_Sequence;
extern volatile unsigned int Timer_Counter;
extern volatile char one_time;
volatile unsigned int ADC_Channel;      // Tracks which channel is being read
volatile unsigned int ADC_Left_Detect;  // Stores Left IR Detector value
volatile unsigned int ADC_Right_Detect; // Stores Right IR Detector value
volatile unsigned int ADC_Thumb;        // Stores Thumbwheel value
char adc_char[4];                       // Stores the ASCII converted BCD values
unsigned int test_value;
char chosen_direction;
char change;

extern unsigned char detect_state;
extern unsigned char follow_state;

unsigned int project_time = 0; // Tracks total ticks
char run_timer = 0;            // Flag to turn timer on/off
extern char adc_char[4];       // From your adc.c file

unsigned int wheel_move;
char forward;

extern volatile char speed;

// project9
unsigned int ring_index = 0;
char ip_address[16];
unsigned int ip_index = 0;
char capturing_ip = 0;
unsigned int parse_state = 0; // State machine tracker

unsigned int time_digits = 0; // Tracks how many time digits we have received

volatile unsigned int iot_movement_timer = 0;

volatile unsigned int wheel_test_timer = 0;
volatile unsigned int s1_timer = 0;
volatile unsigned int iot_startup_state = 0;
unsigned int ok_parse_state = 0;


void project9_state(void) {
  switch (state) {
  case WAIT:
    // [MODIFIED] Project 10 — Show "Waiting for input" after WiFi connects
    strcpy(display_line[0], " Waiting  ");
    strcpy(display_line[1], "for input ");
    strcpy(display_line[2], "          ");
    strcpy(display_line[3], "          ");
    display_changed = TRUE;
    Display_Update(0, 0, 0, 0);

    Init_Serial_UCA0(BAUD_115200);
    Init_Serial_UCA1(BAUD_115200);
    break;

  default:
    break;
  }
}

// void main(void){
void main(void) {
  //    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

  //------------------------------------------------------------------------------
  // Main Program
  // This is the main routine for the program. Execution of code starts here.
  // The operating system is Back Ground Fore Ground.
  //
  //------------------------------------------------------------------------------
  PM5CTL0 &= ~LOCKLPM5;
  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings

  Init_Ports();      // Initialize Ports
  Init_Clocks();     // Initialize Clock System
  Init_Conditions(); // Initialize Variables and Initial Conditions
  Init_Timers();     // Initialize Timers
  Init_LCD();        // Initialize LCD
  Init_ADC();        // ADC Init
  Init_DAC();
  // P2OUT &= ~RESET_LCD;
  //  Place the contents of what you want on the display, in between the quotes
  // Limited to 10 characters per line
  strcpy(display_line[0], "   NCSU   ");
  strcpy(display_line[1], " WOLFPACK ");
  strcpy(display_line[2], "  ECE306  ");
  strcpy(display_line[3], "  GP I/O  ");
  display_changed = TRUE;
  Display_Update(0, 0, 0, 0);

  wheel_move = 0;
  forward = TRUE;
  speed = BAUD_115200;
  state = WAIT;
  project9_state();

  // DRIVE PLEASE

  //------------------------------------------------------------------------------
  // Begining of the "While" Operating System
  //------------------------------------------------------------------------------
  while (ALWAYS) {   // Can the Operating system run
    P2OUT |= IR_LED; // Turn on the IR emitters for Project 6

    motor_safety_check();
    process_iot_buffer();

    // 1. Read and Display Left Detector on Line 1
    //                HEXtoBCD(ADC_Left_Detect);
    //                strcpy(display_line[0], "L:        "); // Setup the
    //                background text adc_line(1, 3);                        //
    //                Drop the numbers in at position 3
    //
    //                // 2. Read and Display Right Detector on Line 2
    //                HEXtoBCD(ADC_Right_Detect);
    //                strcpy(display_line[1], "R:        ");
    //                adc_line(2, 3);
    //
    //                // 3. Read and Display Thumbwheel on Line 3 (Optional, but
    //                very helpful for testing!) HEXtoBCD(ADC_Thumb);
    //                strcpy(display_line[2], "T:        ");
    //                adc_line(3, 3);
    //
    //                display_changed = 1;

// ================================================================
    // [MODIFIED] Project 10 — Seconds counter (20ms tick = 50 ticks/sec)
    // Starts on first IoT command, displays on line 3
    // ================================================================
    if (Last_Time_Sequence != Time_Sequence) {
      Last_Time_Sequence = Time_Sequence;
      cycle_time++;
      time_change = 1;

      if (run_timer == 1) {
        project_time++;

        // Every 50 ticks (1 second at 20ms), update the seconds display
        if ((project_time % 50) == 0) {
            unsigned int seconds = project_time / 50;
            HEXtoBCD(seconds);
            // [MODIFIED] Format: "F0001 999s" — show last cmd + seconds
            display_line[3][5] = ' ';
            display_line[3][6] = adc_char[1]; // Hundreds
            display_line[3][7] = adc_char[2]; // Tens
            display_line[3][8] = adc_char[3]; // Ones
            display_line[3][9] = 's';
            display_line[3][10] = '\0';
            display_changed = TRUE;
        }
      }
    }
switch (state) {
    case 0: // WAIT STATE — idle, only IoT movement commands
      // [FIX] Added break so case 0 doesn't fall through to case 1
      break;

    case 1: // BLACK LINE STATE — runs intercept + follow machines
      black_line_machine();
      line_follow_machine();
      break;
    default:
      Wheels_Stop();
      break;
    }

    // Carlson_StateMachine();            // Run a Time Based State Machine
    Switches_Process();  // Check for switch state change
    Display_Process();   // Update Display
    P3OUT ^= TEST_PROBE; // Change State of TEST_PROBE OFF
  }
  //------------------------------------------------------------------------------
}
