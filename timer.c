/*
 * TimerB0.c
 *
 *  Created on: Feb 25, 2026
 *      Author: K
 */



//------------------------------------------------------------------------------
// Timer B0 initialization sets up both B0_0, B0_1-B0_2 and overflow
#include  "msp430.h"
#include  <string.h>
#include <timer.h>
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"
#include  "dac.h"



extern unsigned int DAC_data;
extern volatile unsigned int Time_Sequence;
unsigned int display_counting = 0;

extern volatile unsigned int transmit_delay_counter;
extern volatile char start_transmission;
extern volatile char iot_TX_buf[16];
extern volatile unsigned int display_index;
extern volatile unsigned int wheel_test_timer;

extern volatile unsigned int iot_startup_timer;
extern volatile unsigned int iot_startup_state;

void Init_Timers(void){
    Init_Timer_B0();
    Init_Timer_B3();
}

volatile unsigned int Timer_Counter;
extern volatile unsigned char update_display;


//project9
volatile unsigned int iot_reset_counter = 0;
volatile char iot_reset_active = 0;
//iot movement
extern volatile unsigned int iot_movement_timer;


void Init_Timer_B0(void) {
    TB0CTL = TBSSEL__SMCLK;     // SMCLK source
    TB0CTL |= TBCLR;            // Resets TB0R
    TB0CTL |= MC__CONTINOUS;    // Continuous up
    TB0CTL |= ID__8;            // Divide clock by 8

    TB0EX0 = TBIDEX__8;         // Divide clock by an additional 8

    TB0CCR0 = TB0CCR0_INTERVAL; // 200ms interval
    TB0CCTL0 |= CCIE;           // Enable CCR0 interrupt

    TB0CCR1 = TB0CCR1_INTERVAL;
    TB0CCTL1 |= CCIE;           // Enable CCR1 interrupt (SW1 Debounce)

    TB0CCR2 = TB0CCR2_INTERVAL;
    TB0CCTL2 |= CCIE;           // Enable CCR2 interrupt (SW2 Debounce)

    TB0CTL &= ~TBIE;            // Disable Overflow Interrupt
    TB0CTL &= ~TBIFG;           // Clear Overflow Interrupt flag
}
//------------------------------------------------------------------------------
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR(void){
//------------------------------------------------------------------------------
// TimerB0 0 Interrupt handler
//----------------------------------------------------------------------------
//...... Add What you need happen in the interrupt ......
 Timer_Counter++;
 Time_Sequence++;   // <--- THIS IS REQUIRED TO UPDATE THE LCD TIMER!
 wheel_test_timer++;
 TB0CCR0 += TB0CCR0_INTERVAL; // Add Offset to TBCCR0
 update_display = TRUE;

 if (display_counting >= 10)
     {
         update_display = TRUE;
         display_counting = 0;
     }

     
// --- PHASE 4: IOT HARDWARE RESET LOGIC ---
    if (iot_reset_active == 1) {
        if (iot_reset_counter > 0) {
            iot_reset_counter--;
        } else {
            P3OUT |= IOT_RN_CPU;    // Set EN back high after 100ms
            iot_reset_active = 0;   // Reset complete
        }
    }
    // -----------------------------------------
    
    // --- PROJECT 9: MOVEMENT COMMAND TIMER ---
    if (iot_movement_timer > 0) {
        iot_movement_timer--;
        
        if (iot_movement_timer == 0) {
            // Time is up! Kill the motors perfectly on time.
            RIGHT_FORWARD_SPEED = WHEEL_OFF;
            LEFT_FORWARD_SPEED  = WHEEL_OFF;
            RIGHT_REVERSE_SPEED = WHEEL_OFF;
            LEFT_REVERSE_SPEED  = WHEEL_OFF;

            // [ADDED] If last command was an arc (A or D), show "Arrived 08"
            // after the movement finishes so the TA can see it
            extern char cmd_direction;
            extern char display_line[4][11];
            extern volatile unsigned char display_changed;
            if (cmd_direction == 'A' || cmd_direction == 'D') {
                display_line[0][0] = 'A';
                display_line[0][1] = 'r';
                display_line[0][2] = 'r';
                display_line[0][3] = 'i';
                display_line[0][4] = 'v';
                display_line[0][5] = 'e';
                display_line[0][6] = 'd';
                display_line[0][7] = ' ';
                display_line[0][8] = '0';
                display_line[0][9] = '8';
                display_changed = 1;
            }


            
        }
    }

    //  if (start_transmission == 1) {
    //      if (transmit_delay_counter > 0) {
    //          transmit_delay_counter--;
    //      } else {
    //          start_transmission = 0; // Stop counting

    //          // Load the string into the buffer (2 spaces between U and #)
    //          strcpy((char*)iot_TX_buf, "NCSU  #1");

    //          // Fire the UCA0 transmit interrupt.
    //          // It will shoot out UCA0, hit your physical J9 jumper,
    //          // bounce back in, and route to UCA1 (Termite) and the LCD!
    //          UCA0IE |= UCTXIE;
    //      }
    //  }
     // -----------------------------------------

     ADCCTL0 |= ADCSC;
 }
//----------------------------------------------------------------------------

#pragma vector=TIMER0_B1_VECTOR
__interrupt void TIMER0_B1_ISR(void) {
    switch (__even_in_range(TB0IV, 14)) {
        case 0: break;
        case 2: // CCR1 (SW1 Debounce)
            TB0CCTL1 &= ~CCIE;
            TB0CCR1 += TB0CCR1_INTERVAL;
            P4IFG &= ~SW1;
            P4IE |= SW1;                  // Turn SW1 interrupt back on
            break;
        case 4: // CCR2 (SW2 Debounce)
            TB0CCTL2 &= ~CCIE;
            TB0CCR2 += TB0CCR2_INTERVAL;
            P2IFG &= ~SW2;
            P2IE |= SW2;                  // Turn SW2 interrupt back on
            break;
        case 14: // overflow
        DAC_data = DAC_data - 100;
        SAC3DAT = DAC_data; // Initial DAC data
        if (DAC_data <= DAC_Limit)
        {
            DAC_data = DAC_Adjust;
            SAC3DAT = DAC_data; // Initial DAC data
            TB0CTL &= ~TBIE; // disable TimerB0 overflow interrupt
            // RED_LED_OFF; // Set RED_LED Off DAC voltage
         }
        default: break;
    }
}

void Init_Timer_B3(void)
{
//------------------------------------------------------------------------------
// SMCLK source, up count mode, PWM Right Side
// TB3.1 P6.0 LCD_BACKLITE
// TB3.2 P6.1 R_FORWARD
// TB3.3 P6.2 R_REVERSE
// TB3.4 P6.3 L_FORWARD
// TB3.5 P6.4 L_REVERSE
//------------------------------------------------------------------------------
    TB3CTL = TBSSEL__SMCLK; // SMCLK
    TB3CTL |= MC__UP; // Up Mode
    TB3CTL |= TBCLR; // Clear TAR
    PWM_PERIOD = WHEEL_PERIOD; // PWM Period [Set this to 50005]
    TB3CCTL1 = OUTMOD_7; // CCR1 reset/set
    LCD_BACKLITE_DIMING = PERCENT_80; // P6.0 Right Forward PWM duty cycle


    TB3CCTL2 = OUTMOD_7; // CCR2 reset/set
    RIGHT_FORWARD_SPEED = WHEEL_OFF; // P6.1 Right Forward PWM duty cycle


    TB3CCTL3 = OUTMOD_7; // CCR3 reset/set
    LEFT_FORWARD_SPEED = WHEEL_OFF; // P6.2 Left Forward PWM duty cycle


    TB3CCTL4 = OUTMOD_7; // CCR4 reset/set
    RIGHT_REVERSE_SPEED = WHEEL_OFF; // P6.3 Right Reverse PWM duty cycle


    TB3CCTL5 = OUTMOD_7; // CCR5 reset/set
    LEFT_REVERSE_SPEED = WHEEL_OFF; // P6.4 Left Reverse PWM duty cycle


//------------------------------------------------------------------------------
}



