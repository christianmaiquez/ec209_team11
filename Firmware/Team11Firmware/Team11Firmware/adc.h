#ifndef _ADC_H
#define _ADC_H

#include <stdint.h>

void     adc_init(void);
uint16_t adc_read(uint8_t chan);          // blocking single read (polling)
uint16_t adc_convert_mv(uint16_t value);  // raw count -> mV

// Captures n_pairs alternating samples (v = ADC0, i = ADC1), 100us apart,
// Timer0-triggered, interrupt-driven. Results are in mV. Needs sei().
void     adc_capture(uint16_t *v, uint16_t *i, uint8_t n_pairs);

#endif