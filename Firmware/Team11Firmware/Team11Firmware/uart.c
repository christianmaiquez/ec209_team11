#include "common.h"
#include "uart.h"
#include <avr/io.h>
#include "calculations.h"

#define BAUD       9600UL
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)   // = 12 at 2MHz

void uart_init(void)
{
	UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
	UBRR0L = (uint8_t)UBRR_VALUE;
	UCSR0B = (1 << TXEN0);                     // transmit only
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);    // 8 data bits, 1 stop bit, no parity
}

void uart_putc(char c)
{
	while (!(UCSR0A & (1 << UDRE0))) {}
	UDR0 = c;
}

void uart_puts(const char *s)
{
	while (*s) {
		uart_putc(*s++);
	}
}

void uart_put_uint32(uint32_t value)
{
	char digits[10];
	uint8_t n = 0;

	if (value == 0) {
		uart_putc('0');
		return;
	}
	while (value > 0) {
		digits[n++] = '0' + (value % 10);
		value /= 10;
	}
	while (n > 0) {
		uart_putc(digits[--n]);
	}
}

void uart_put_uint(uint16_t value)
{
	uart_put_uint32(value);
}

void uart_put_float2(float x)
{
	if (x < 0.0f) {
		uart_putc('-');
		x = -x;
	}
	uint32_t hundredths = (uint32_t)(x * 100.0f + 0.5f);
	uart_put_uint32(hundredths / 100);
	uart_putc('.');
	uint8_t frac = hundredths % 100;
	uart_putc('0' + frac / 10);
	uart_putc('0' + frac % 10);
}


void uart_print_report(void)
{
	uart_puts("Vdc(pin) = "); uart_put_float2(measure_v_dc_mv()); uart_puts(" mV   ");
	uart_puts("Idc(pin) = "); uart_put_float2(measure_i_dc_mv()); uart_puts(" mV\r\n");

	uart_puts("Vrms = "); uart_put_float2(measure_vrms_v());  uart_puts(" V\r\n");
	uart_puts("Ipk  = "); uart_put_float2(measure_ipk_a());   uart_puts(" A\r\n");
	uart_puts("P    = "); uart_put_float2(measure_power_w());  uart_puts(" W\r\n\r\n");
}