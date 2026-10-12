#include "common.h"
#include <avr/interrupt.h>
#include "adc.h"
#include "uart.h"
#include "calculations.h"

int main(void)
{
	uint16_t v[SAMPLE_COUNT + 1];
	uint16_t i[SAMPLE_COUNT + 1];

	uart_init();
	adc_init();
	sei();

	while (1) {
		adc_capture(v, i, SAMPLE_COUNT + 1);

		float vrms_mv = calc_rms(v, SAMPLE_COUNT) * V_GAIN;     // mV at source
		float ipk_ma  = calc_peak(i, SAMPLE_COUNT) * I_GAIN;    // mA at load
		float p_mw    = calc_power(v, i, SAMPLE_COUNT);         // mW

		uart_puts("Vrms: "); uart_put_float1(vrms_mv / 1000.0f); uart_puts(" V\r\n");
		uart_puts("Ipk:  "); uart_put_float1(ipk_ma);            uart_puts(" mA\r\n");
		uart_puts("P:    "); uart_put_float1(p_mw);              uart_puts(" mW\r\n\r\n");

		for (volatile uint32_t d = 0; d < 100000UL; d++) {}     // crude delay
	}
}