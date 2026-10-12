#include "common.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include "adc.h"
#include "calculations.h"
#include "display.h"

#define SHOW_TIME_MS   1500  // how long each quantity stays on the display

int main(void)
{
	uint16_t v[SAMPLE_COUNT];      // 40 voltage samples, mV at the ADC pin
	uint16_t i[SAMPLE_COUNT];      // 40 current samples, mV at the ADC pin

	init_display();
	adc_init();
	sei();

	while (1) {
		// Before each quantity is shown, take a fresh set of 40 samples
		for (uint8_t quantity = 0; quantity < 3; quantity++) {
			#if USE_FAKE_DATA
			for (uint8_t k = 0; k < SAMPLE_COUNT; k++) {
				v[k] = 2500 + 2000 * sinf(2 * M_PI * k / 10.0f);
				i[k] = 2500 + 1000 * sinf(2 * M_PI * (k + 0.5f) / 10.0f);
			}
			#else
			adc_capture(v, i, SAMPLE_COUNT);
			#endif

			if (quantity == 0) {
				float vrms_v = calc_rms(v, SAMPLE_COUNT) * V_GAIN / 1000.0f;    // V
				display_show_number(vrms_v, SEG_U);
				} else if (quantity == 1) {
				float ipk_a = calc_peak(i, SAMPLE_COUNT) * I_GAIN / 1000.0f;    // A
				display_show_number(ipk_a, SEG_A);
				} else {
				float p_w = calc_power(v, i, SAMPLE_COUNT) / 1000.0f;           // W
				display_show_number(p_w, SEG_P);
			}

			_delay_ms(SHOW_TIME_MS);
		}
	}
}