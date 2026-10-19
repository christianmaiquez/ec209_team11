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
	// Sets up UART, ADC and Display
	uart_init();
	display_init();
	adc_init();
	sei(); // Turns on global interrupts

	while (1) {
		measure_capture();       // 40 fresh samples of v and i
		uart_print_report();     // Prints Vrms, Ipk and P to terminal
		display_show_next();     // Shows either U, A or P on the display depending on whats next
		_delay_ms(SHOW_TIME_MS); 
	}
}