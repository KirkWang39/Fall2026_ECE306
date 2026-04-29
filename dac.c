//------------------------------------------------------------------------------
//  MSP430FR235x Demo - SAC-L3, DAC Buffer Mode
//
//  Description: Configure SAC-L3 for DAC Buffer Mode. Use the 12 bit DAC to
//  output positive ramp. The OA is set in buffer mode to improve DAC output
//  drive strength. Internal 2.5V reference is selected as DAC reference.
//  Observe the output of OA0O pin with oscilloscope.
//  ACLK = n/a, MCLK = SMCLK = default DCODIV ~1MHz.
//
//                MSP430FR235x
//             -------------------
//         /|\|                   |
//          | |                   |
//          --|RST     DAC12->OA0O|--> oscilloscope
//            |                   |
//            |                   |
//------------------------------------------------------------------------------
#include  "msp430.h"
#include  <string.h>
#include <timer.h>
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"
#include "adc.h"
#include "dac.h"

unsigned int DAC_data;

void Init_DAC(void){

    SAC3DAC = DACSREF_0;                // Select VCC as DAC reference
    SAC3DAC |= DACLSEL_0;                // DAC latch loads when DACDAT written

    SAC3OA = NMUXEN;                   // SAC Negative input MUX control
    SAC3OA |= PMUXEN;                   // SAC Positive input MUX control
    SAC3OA |= PSEL_1;                   // 12-bit reference DAC source selected
    SAC3OA |= NSEL_1;                   // Select negative pin input
    SAC3OA |= OAPM;                     // Select low speed and low power mode
    SAC3PGA = MSEL_1;                   // Set OA as buffer mode
    SAC3OA |= SACEN;                    // Enable SAC
    SAC3OA |= OAEN;                     // Enable OA

    DAC_data = DAC_Begin;              // Starting Low value for DAC output [2v]
    SAC3DAT = DAC_data;                  // Initial DAC data

    TB0CTL |= TBIE;                      // Timer B0 overflow interrupt enable

    //RED_LED_ON;                          // Set RED_LED On indicates the OVERFLOW timer start

    SAC3DAC |= DACEN;                    // Enable DAC

}
//
//// The following line should be done in a timer overflow interrupt [after 2 or 3 overflows]
//  P2OUT   |=  DAC_ENB;                  // Value = High [enabled]
//// Each time through the overflow time after enable, subtract 50 from DAC_data
//  DAC_data = 4000;
//  SAC3DAT  = DAC_data;                  // Stepping DAC Output
//// Somewhere around 1200 will be about 6v. You will need to measure it.
//
//
//// The interrupt is not used
//#pragma vector = SAC1_SAC3_VECTOR
//__interrupt void SAC3_ISR(void){
//  switch(__even_in_range(SAC0IV,SACIV_4)){
//  case SACIV_0: break;
//  case SACIV_2: break;
//  case SACIV_4:
////   DAC_data++;
////   DAC_data &= 0xFFF;
////   SAC3DAT = DAC_data;                 // DAC12 output positive ramp
//    break;
//  default: break;
//  }
//}
