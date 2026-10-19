#include "calculations.h"
#include <math.h>
#include "adc.h"

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
float calc_peak_from_rms(const uint16_t *voltage, uint8_t number_of_samples)
{
	return calc_rms(voltage, number_of_samples) * SQUARE_ROOT_OF_2;
}

// Largest AC excursion actually sampled. With ~10 samples per cycle this can
// read up to ~5% low because a sample may miss the crest.
float calc_peak_sampled(const uint16_t *voltage, uint8_t number_of_samples)
{
	float offset = average(voltage, number_of_samples);
	float peak = 0;

	for (uint8_t sample = 0; sample < number_of_samples; sample++) {
		float ac_part = fabsf(voltage[sample] - offset);
		if (ac_part > peak) {
			peak = ac_part;
		}
	}

	return peak;
}

// Real power = average of (voltage x current) with both offsets removed.
// Both arrays have number_of_samples entries. Because the capture covers whole
// cycles, the sample after voltage[n-1] is voltage[0], so the index wraps.
float calc_power(const uint16_t *voltage, const uint16_t *current,
uint8_t number_of_samples)
{
	float voltage_offset = average(voltage, number_of_samples);
	float current_offset = average(current, number_of_samples);
	float cos_theta = cosf(PHASE_RAD);
	float total_of_products = 0;

	for (uint8_t sample = 0; sample < number_of_samples; sample++) {
		uint8_t next = (sample + 1 < number_of_samples) ? sample + 1 : 0;

		// current[sample] was taken between voltage[sample] and voltage[next]
		float voltage_sum = (float)voltage[sample] + voltage[next]
		- 2.0f * voltage_offset;
		float current_ac_part = current[sample] - current_offset;

		total_of_products += voltage_sum * current_ac_part;
	}

	// Factor 2 (two voltage samples summed) and cos(theta) correction applied once
	float average_power = total_of_products / (2.0f * cos_theta * number_of_samples);  // mV x mV

	// mV (source) x mA (load) = uW, divide by 1000 for mW
	return average_power * V_GAIN * I_GAIN / 1000.0f;
}


// ---------------------------------------------------------------------------
// Measurement: capture + results
// ---------------------------------------------------------------------------
static uint16_t sample_v[SAMPLE_COUNT];   // voltage samples, mV at the ADC pin
static uint16_t sample_i[SAMPLE_COUNT];   // current samples, mV at the ADC pin

void measure_capture(void)
{
	#if USE_FAKE_DATA
	// Computed test signals: 500Hz, 10 samples per cycle, 2.5V offset
	for (uint8_t k = 0; k < SAMPLE_COUNT; k++) {
		sample_v[k] = 2500 + 2000 * sinf(2 * M_PI * k / 10.0f);
		sample_i[k] = 2500 + 1000 * sinf(2 * M_PI * (k + 0.5f) / 10.0f);
	}
	#else
	adc_capture(sample_v, sample_i, SAMPLE_COUNT);
	#endif
}

float measure_vrms_v(void)         { return calc_rms(sample_v, SAMPLE_COUNT) * V_GAIN / 1000.0f; }
float measure_ipk_a(void)          { return calc_peak_from_rms(sample_i, SAMPLE_COUNT) * I_GAIN / 1000.0f; }
float measure_ipk_sampled_a(void)  { return calc_peak_sampled(sample_i, SAMPLE_COUNT) * I_GAIN / 1000.0f; }
float measure_power_w(void)        { return calc_power(sample_v, sample_i, SAMPLE_COUNT) / 1000.0f; }
float measure_v_dc_mv(void)        { return average(sample_v, SAMPLE_COUNT); }
float measure_i_dc_mv(void)        { return average(sample_i, SAMPLE_COUNT); }