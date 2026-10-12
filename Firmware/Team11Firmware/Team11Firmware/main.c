#include "common.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include "adc.h"
#include "uart.h"
#include "display.h"
#include "calculations.h"

#define SHOW_TIME_MS  1000   // how long each quantity stays on the display

int main(void)
{
	uart_init();
	init_display();
	adc_init();
	sei();

	while (1) {
		measure_capture();       // 40 fresh samples of v and i
		uart_print_report();     // terminal: V, I and P
		display_show_next();     // 7-segment: next of U, A, P
		_delay_ms(SHOW_TIME_MS);
	}
}