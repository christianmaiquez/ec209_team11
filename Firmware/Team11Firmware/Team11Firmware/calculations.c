#include "calculations.h"
#include <math.h>

#define SQUARE_ROOT_OF_2  1.41421356f

// Average of the samples (the DC offset), removed later to leave only the AC part.
// Summing in a uint32_t is exact: 255 samples x 5000 mV fits easily, and it avoids
// slow software-float additions on the AVR.
static float average(const uint16_t *samples, uint8_t number_of_samples)
{
	uint32_t total = 0;

	for (uint8_t sample = 0; sample < number_of_samples; sample++) {
		total += samples[sample];
	}

	return (float)total / number_of_samples;
}

// RMS of the AC part (offset removed), in mV at the ADC pin
float calc_rms(const uint16_t *voltage, uint8_t number_of_samples)
{
	float offset = average(voltage, number_of_samples);
	float total_of_squares = 0;

	for (uint8_t sample = 0; sample < number_of_samples; sample++) {
		float ac_part = voltage[sample] - offset;
		total_of_squares += ac_part * ac_part;
	}

	return sqrtf(total_of_squares / number_of_samples);
}

// Peak of a sine wave from its RMS: Vp = Vrms * sqrt(2).
// With only ~9.6 samples per cycle, the largest sample usually misses the true peak,
// so this is more accurate than taking the maximum sample.
float calc_peak(const uint16_t *voltage, uint8_t number_of_samples)
{
	return calc_rms(voltage, number_of_samples) * SQUARE_ROOT_OF_2;
}

// Real power = average of (voltage x current) with both offsets removed.
// voltage has number_of_samples + 1 entries, current has number_of_samples.
float calc_power(const uint16_t *voltage, const uint16_t *current,
uint8_t number_of_samples)
{
	float voltage_offset = average(voltage, number_of_samples);
	float current_offset = average(current, number_of_samples);
	float total_of_products = 0;

	for (uint8_t sample = 0; sample < number_of_samples; sample++) {
		// Current[sample] was taken between voltage[sample] and voltage[sample + 1], so the
		// voltage at that moment is estimated from their average. For a sine wave, that
		// average is too small by a factor of COS_THETA, which is corrected below.
		float voltage_sum = (float)voltage[sample] + voltage[sample + 1]
		- 2.0f * voltage_offset;
		float current_ac_part = current[sample] - current_offset;

		total_of_products += voltage_sum * current_ac_part;
	}

	// Apply the factor of 2 (from summing two voltage samples) and the COS_THETA
	// correction once, instead of every time round the loop.
	float average_power = total_of_products / (2.0f * COS_THETA * number_of_samples);  // mV x mV

	// Apply the gains: mV (source) x mA (load) = uW, then divide by 1000 to get mW
	return average_power * V_GAIN * I_GAIN / 1000.0f;
}