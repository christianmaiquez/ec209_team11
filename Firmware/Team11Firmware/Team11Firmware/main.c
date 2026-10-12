#include "common.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include "adc.h"
#include "calculations.h"
#include "display.h"

int main(void)
{
	uint16_t v[SAMPLE_COUNT];
	uint16_t i[SAMPLE_COUNT];
	uint8_t  loops = 0;

	init_display();
	adc_init();
	sei();

	while (1) {
		adc_capture(v, i, SAMPLE_COUNT);

		float vrms_v = calc_rms(v, SAMPLE_COUNT) * V_GAIN / 1000.0f;   // V
		float ipk_a  = calc_peak(i, SAMPLE_COUNT) * I_GAIN / 1000.0f;  // A
		float p_w    = calc_power(v, i, SAMPLE_COUNT) / 1000.0f;       // W

		// Rotate through the quantities, about 2 s each
		switch ((loops / 20) % 3) {
			case 0:  display_show_number(vrms_v, SEG_U); break;
			case 1:  display_show_number(ipk_a,  SEG_A); break;
			default: display_show_number(p_w,    SEG_P); break;
		}
		loops++;

		_delay_ms(100);
	}
}