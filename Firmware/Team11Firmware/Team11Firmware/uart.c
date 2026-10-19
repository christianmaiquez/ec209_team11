#include "common.h"
#include "uart.h"
#include <avr/io.h>
#include "calculations.h"

#define BAUD 9600UL
#define UBRR_VALUE ((F_CPU + 8UL * BAUD) / (16UL * BAUD) - 1) // Baud divisor for normal-speed mode (12 at 2MHz)

// This function sets up the UART with 9600 baud
void uart_init(void)
{
	UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
	UBRR0L = (uint8_t)UBRR_VALUE;
	UCSR0B = (1 << TXEN0); // Transmitter is enabled. Nothing is received, so no need for receiver.
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bits, no parity, 1 stop bit
}

// This helper function sends one character to the UART data register
static void uart_putc(char c)
{
	while (!(UCSR0A & (1 << UDRE0))) {} // Waits until the transmit buffer is free
	UDR0 = c; // Then queues the byte
}

// This helper function sends a null-terminated string until it reaches the end of the message.
static void uart_puts(const char *s)
{
	while (*s) {
		uart_putc(*s++);
	}
}


// This helper function sends an unsigned integer in decimal, without leading zeros.
static void put_uint16(uint16_t value)
{
	char digits[5];                         // largest uint16_t (65535) has 5 digits
	uint8_t n = 0;

	do {
		digits[n++] = '0' + (value % 10);   // Converts digits to ASCII character for UART
		value /= 10; // Integer division drops the last digit
	} while (value > 0);

	// Digits come out lowest first, so they are stored and sent in reverse.
	while (n > 0) {
		uart_putc(digits[--n]);
	}
}

// This helper function sends a float as text with 2 decimal places
static void uart_put_float2(float x)
{
	if (x < 0.0f)    x = 0.0f;                              // power can dip below 0 from noise
	if (x > 655.35f) x = 655.35f;                           // limit to what a uint16_t can hold
	uint16_t hundredths = (uint16_t)(x * 100.0f + 0.5f);    // +0.5 rounds to the nearest hundredth

	put_uint16(hundredths / 100);               // whole part
	uart_putc('.');
	uart_putc('0' + (hundredths % 100) / 10);   // tenths
	uart_putc('0' + hundredths % 10);           // hundredths
}

// SENDING VALUES THROUGH TO TERMINAL

// This is a helper function to format our values easily.
static void print_value(const char *label, float value, const char *unit)
{
	uart_puts(label);
	uart_put_float2(value);
	uart_puts(unit);
}

// This function sends the results of the latest measure_capture() to the terminal
void uart_print_report(void)
{
	// Calls upon the helper function print_value instead of printing each section separately
	print_value("Vrms = ", measure_vrms_v(),  " V\r\n");
	print_value("Ipk  = ", measure_ipk_a(),   " A\r\n");
	print_value("P    = ", measure_power_w(), " W\r\n\r\n");
}