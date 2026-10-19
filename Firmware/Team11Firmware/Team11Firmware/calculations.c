#include "calculations.h"
#include <math.h>
#include <stdint.h>
#include "adc.h"

// Defining constant 
#define V_GAIN        24.53f   // Gain from voltage divider and differential amplifier 
#define I_GAIN        1.127f   // Gain from current sensor and differential amplifier
#define PHASE_SHIFT    0.9511f  // The phase shift between voltage and current, calculated from cos(2*pi*f*T)
#define SAMPLE_COUNT  40 // Samples per channel
#define SQUARE_ROOT_OF_2  1.41421356f

// Declares the ADC array to take the voltage and current samples alternately
static uint16_t sample_v[SAMPLE_COUNT];
static uint16_t sample_i[SAMPLE_COUNT];

// This helper function returns the average of a channel's samples. For an AC signal that
// is the DC offset (the bias the signal sits on)
static float average(const uint16_t *samples)
{
	uint32_t total = 0;

	// Add all the ADC reading values together
	for (uint8_t k = 0; k < SAMPLE_COUNT; k++) {
		total += samples[k];
	}

	return (float)total / SAMPLE_COUNT; // Divide it by how much samples were taken
}

// This helper function returns the RMS of the AC part of a channel.
static float calc_rms(const uint16_t *samples)
{
	float offset = average(samples);
	float total_of_squares = 0;

	for (uint8_t k = 0; k < SAMPLE_COUNT; k++) {
		float ac_part = samples[k] - offset; // It subtracts the offset first
		total_of_squares += ac_part * ac_part; // Then takes the square root of the average of the squares.
	}

	return sqrtf(total_of_squares / SAMPLE_COUNT); // Takes rms using sqrtf function from math library
}

// This helper function returns the real power. P = IV
// The voltage at the currents instant is estimated through linear approximation
static float calc_power(void)
{
	float voltage_offset = average(sample_v);
	float current_offset = average(sample_i);
	float total_of_products = 0;

	for (uint8_t current = 0; current < SAMPLE_COUNT; current++) {
		// With linear approximation, there is no neighboring points for first and last samples
		uint8_t next = current + 1;
		if (next >= SAMPLE_COUNT) next = 0; // Last sample is approximated with first sample

		// Sum of the two neighboring voltage samples with the offset removed 
		float voltage_sum = (float)sample_v[current] + sample_v[next] - 2.0f * voltage_offset;
		float current_ac_part = sample_i[current] - current_offset;

		total_of_products += voltage_sum * current_ac_part; 
	}

	// Divide by 2 (the voltage sum is twice the average), COS_THETA, and the number of samples
	return total_of_products / (2.0f * PHASE_SHIFT * SAMPLE_COUNT);
}

// This function takes voltage and current samples from the adc
void measure_capture(void) 
{
	adc_capture(sample_v, sample_i, SAMPLE_COUNT);
}

// The results below convert mV at the ADC pin into real units using the gains above.
// RMS voltage at the source, in V
float measure_vrms_v(void) 
{ 
	return calc_rms(sample_v) * V_GAIN / 1000.0f;
}

// Peak current in the load, in A. For a sine wave, peak = RMS x sqrt(2)
float measure_ipk_a(void) 
{ 
	return calc_rms(sample_i) * SQUARE_ROOT_OF_2 * I_GAIN / 1000.0f; 
}

// Real power in W. mV (source) x mA (load) = uW, so divide by 1000 twice to get W
float measure_power_w(void) 
{
	return calc_power() * V_GAIN * I_GAIN / 1.0e6f;
}