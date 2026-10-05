#include "common.h"
#include "uart.h"
#include <avr/io.h>

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
	while (!(UCSR0A & (1 << UDRE0))) {}        // wait for empty data register
	UDR0 = c;
}

void uart_puts(const char *s)
{
	while (*s) {
		uart_putc(*s++);
	}
}

void uart_put_uint(uint16_t value)
{
	char digits[5];
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