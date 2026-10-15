#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void adc_init(void);
void adc_capture(uint16_t *v, uint16_t *i, uint8_t n_pairs);

#endif