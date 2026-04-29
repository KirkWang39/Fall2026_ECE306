/*
 * TImer.h
 *
 *  Created on: Feb 25, 2026
 *      Author: k
 */

#ifndef TIMER_H_
#define TIMER_H_
#define TB0CCR0_INTERVAL (2500) // 20ms
#define TB0CCR1_INTERVAL (12500) // 50ms debounce for SW1
#define TB0CCR2_INTERVAL (12500) // 50ms debounce for SW2


void Init_Timer_B0(void);
void Init_Timer_B3(void);


#endif /* TIMER_H_ */
