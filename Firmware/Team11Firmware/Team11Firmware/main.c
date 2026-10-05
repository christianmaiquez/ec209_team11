#include "common.h"
#include <avr/interrupt.h>
#include "adc.h"
#include "uart.h"

int main(void)
{
	uart_init();
	adc_init();
	sei();

	adc_capture_and_print();

	while (1);
}


