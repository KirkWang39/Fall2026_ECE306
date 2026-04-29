/*
 * adc.h
 *
 *  Created on: Mar 8, 2026
 *      Author: Justin Banh
 */

#ifndef ADC_H_
#define ADC_H_

void Init_ADC(void);
void HEXtoBCD(int hex_value);
void adc_line(char line, char location);

#endif /* ADC_H_ */
