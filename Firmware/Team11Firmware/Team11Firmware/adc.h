#ifndef _ADC_H
#define _ADC_H

#include <stdint.h>

void     adc_init(void);
uint16_t adc_read(uint8_t chan);
uint16_t adc_convert_mv(uint16_t value);

// Captures n_pairs alternating samples (v = ADC0, i = ADC1), 104us apart,
// timer-triggered. Results are returned in mV. Needs sei().
void     adc_capture(uint16_t *v, uint16_t *i, uint8_t n_pairs);

#endif