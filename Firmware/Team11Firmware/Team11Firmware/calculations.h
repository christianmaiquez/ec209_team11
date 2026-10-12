#ifndef CALCULATIONS_H
#define CALCULATIONS_H

#include <stdint.h>

// ---- Change these to match your hardware ----
#define V_GAIN                 14.1f    // source volts per ADC volt
#define I_GAIN                 2.0f     // load amps per ADC volt (= mA per mV)
#define COS_THETA              0.9511f  // cos(2*pi*f*Ts): 500Hz, 100us between a V sample and the next I sample
#define SAMPLES_PER_CYCLE_X10  100      // 500Hz and 200us per sample of one channel = 10.0

// Samples per channel. Must be a whole number of signal cycles.
#define SAMPLE_COUNT           40       // 40 / 10 = 4 cycles

_Static_assert((SAMPLE_COUNT * 10) % SAMPLES_PER_CYCLE_X10 == 0,
"SAMPLE_COUNT must be a whole number of signal cycles");

// Samples are in mV at the ADC pin, taken alternately: v[0], i[0], v[1], i[1], ...
// v[] and i[] both have SAMPLE_COUNT entries.

float calc_rms(const uint16_t *v, uint8_t n);    // mV at the ADC pin (offset removed)
float calc_peak(const uint16_t *v, uint8_t n);   // mV at the ADC pin (offset removed)

// v and i both have n entries. Returns mW at the source.
float calc_power(const uint16_t *v, const uint16_t *i, uint8_t n);

#endif