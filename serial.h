/*
 * serial.h
 *
 *  Created on: Mar 26, 2026
 *      Author: Justin Banh
 */

#ifndef SERIAL_H_
#define SERIAL_H_

//------------------------------------------------------------------------------
// Extern Declarations (Variables defined in serial.c)
//------------------------------------------------------------------------------
extern volatile char IOT_Ring_Rx[RING_BUF_SIZE];
extern volatile char USB_Ring_Rx[RING_BUF_SIZE];
extern volatile char iot_TX_buf[RING_BUF_SIZE];

extern volatile unsigned int iot_rx_wr;
extern volatile unsigned int usb_rx_wr;
extern volatile unsigned int iot_tx;
extern volatile unsigned int direct_iot;
extern volatile unsigned int display_index;

// HW8 2-Second Delay Variables (Shared between switches.c, TimerB0.c, and serial.c)


//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------
void Init_Serial_UCA0(char speed);
void Init_Serial_UCA1(char speed);

void Send_IoT_Command(char* cmd);
void Process_IoT_Startup(void);
void process_iot_buffer(void);

#endif /* SERIAL_H_ */
