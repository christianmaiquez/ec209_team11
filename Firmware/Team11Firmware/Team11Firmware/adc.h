#ifndef _ADC_H
#define _ADC_H

#include <stdint.h>

void     adc_init(void);
uint16_t adc_read(uint8_t chan);          // blocking single read (polling)
uint16_t adc_convert_mv(uint16_t value);  // raw count -> mV

// Captures 40 samples each from ADC0 and ADC1 (alternating, interrupt-driven),
// then prints them over UART as "ADC0, ADC1" rows in mV.
// Requires interrupts enabled (sei()) and uart_init() to have been called.
void     adc_capture_and_print(void);

#endif