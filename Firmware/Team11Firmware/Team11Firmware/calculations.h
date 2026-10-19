#ifndef CALCULATIONS_H
#define CALCULATIONS_H

#include <stdint.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979f
#endif

// ---- Change these to match your hardware ----
#define V_GAIN                 24.53f   // source volts per ADC volt -closer look at this needed
#define I_GAIN                 1.127f     // load amps per ADC volt (= mA per mV)
#define SAMPLES_PER_CYCLE_X10  100      // 500Hz and 200us per sample of one channel = 10.0

// Phase between a V sample and the next I sample (half a sample period):
// theta = pi / samples_per_cycle. 100us at 500Hz = 18 degrees.
#define PHASE_RAD  (M_PI * 10.0f / SAMPLES_PER_CYCLE_X10)

// Samples per channel. Must be a whole number of signal cycles.
#define SAMPLE_COUNT           40       // 40 / 10 = 4 cycles

_Static_assert((SAMPLE_COUNT * 10) % SAMPLES_PER_CYCLE_X10 == 0,
"SAMPLE_COUNT must be a whole number of signal cycles");

// Samples are in mV at the ADC pin, taken alternately: v[0], i[0], v[1], i[1], ...
// v[] and i[] both have SAMPLE_COUNT entries.

float calc_rms(const uint16_t *v, uint8_t n);             // mV at the ADC pin (offset removed)
float calc_peak_from_rms(const uint16_t *v, uint8_t n);   // Vrms * sqrt(2), assumes a clean sine
float calc_peak_sampled(const uint16_t *v, uint8_t n);    // largest AC excursion actually sampled

// v and i both have n entries. Returns mW at the source.
float calc_power(const uint16_t *v, const uint16_t *i, uint8_t n);


// ---- Measurement: capture samples and get the results (uses the ADC) ----
#define USE_FAKE_DATA  0     // 1 = computed sine waves, 0 = real ADC samples

void  measure_capture(void);     // take a fresh set of SAMPLE_COUNT v and i samples

// Results from the most recent capture
float measure_vrms_v(void);        // RMS voltage at the source, V
float measure_ipk_a(void);         // peak load current, A
float measure_ipk_sampled_a(void); // peak load current from raw samples, A (distortion check)
float measure_power_w(void);       // average real power, W
float measure_v_dc_mv(void);       // average level at the ADC pin, mV (0 = nothing connected)
float measure_i_dc_mv(void);

#endif