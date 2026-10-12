#ifndef CALCULATIONS_H
#define CALCULATIONS_H

#include <stdint.h>

// ---- Change these to match your hardware ----
#define V_GAIN                 14.1f    // source volts per ADC volt
#define I_GAIN                 2.0f     // load amps per ADC volt (= mA per mV)
#define COS_THETA              0.9471f  // cos(2*pi*f*Ts): 500Hz, 104us between a V sample and the
// next I sample (half the 208us V-to-V spacing).
// Set to 1.0f for plain linear interpolation.
#define SAMPLES_PER_CYCLE_X10  96       // 500Hz and 208us per sample of one channel = 9.6

// Number of samples used per channel in every calculation.
// Must cover a whole number of signal cycles so a partial cycle doesn't bias the result.
#define SAMPLE_COUNT           48       // 48 / 9.6 = 5 cycles

// Compile-time check: SAMPLE_COUNT must be a whole number of cycles
_Static_assert((SAMPLE_COUNT * 10) % SAMPLES_PER_CYCLE_X10 == 0,
"SAMPLE_COUNT must be a whole number of signal cycles");

// Samples are in mV at the ADC pin, taken alternately: v[0], i[0], v[1], i[1], ...
//
// Buffer sizes:  v[] needs SAMPLE_COUNT + 1 entries  (calc_power interpolates between
//                                                    v[k] and v[k+1], so it needs one extra)
//                i[] needs SAMPLE_COUNT entries
// So capture SAMPLE_COUNT + 1 pairs and ignore the last i[].

float calc_rms(const uint16_t *v, uint8_t n);    // mV at the ADC pin (offset removed)
float calc_peak(const uint16_t *v, uint8_t n);   // mV at the ADC pin (offset removed)

// v has n + 1 valid entries, i has n. Returns mW at the source.
float calc_power(const uint16_t *v, const uint16_t *i, uint8_t n);

#endif  // CALCULATIONS_H 