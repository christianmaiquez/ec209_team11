#include "calculations.h"
#include <math.h>

#define SQUARE_ROOT_OF_2  1.41421356f

// Average of the samples (the DC offset)
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

// Peak of a sine wave from its RMS: Vp = Vrms * sqrt(2)
float calc_peak(const uint16_t *voltage, uint8_t number_of_samples)
{
	return calc_rms(voltage, number_of_samples) * SQUARE_ROOT_OF_2;
}

// Real power = average of (voltage x current) with both offsets removed.
// Both arrays have number_of_samples entries. Because the capture covers whole
// cycles, the sample after voltage[n-1] is voltage[0], so the index wraps.
float calc_power(const uint16_t *voltage, const uint16_t *current,
uint8_t number_of_samples)
{
	float voltage_offset = average(voltage, number_of_samples);
	float current_offset = average(current, number_of_samples);
	float total_of_products = 0;

	for (uint8_t sample = 0; sample < number_of_samples; sample++) {
		uint8_t next = (sample + 1 < number_of_samples) ? sample + 1 : 0;

		// current[sample] was taken between voltage[sample] and voltage[next]
		float voltage_sum = (float)voltage[sample] + voltage[next]
		- 2.0f * voltage_offset;
		float current_ac_part = current[sample] - current_offset;

		total_of_products += voltage_sum * current_ac_part;
	}

	// Factor 2 (two voltage samples summed) and COS_THETA correction applied once
	float average_power = total_of_products / (2.0f * COS_THETA * number_of_samples);  // mV x mV

	// mV (source) x mA (load) = uW, divide by 1000 for mW
	return average_power * V_GAIN * I_GAIN / 1000.0f;
}