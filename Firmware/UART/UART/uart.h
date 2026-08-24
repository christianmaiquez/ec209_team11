/*
 * uart.h
 *
 * Contains the function declarations required to configure
 * USART0, transmit data, extract digits from three-digit
 * numbers, and convert numerical digits into ASCII values.
 *
 * Author: Team 11
 */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>

/* Initialise UART for 9600 baud, 8 data bits, no parity, 1 stop bit. */
void uart_init(void);

/* Transmits one byte through UART. */
void uart_transmit_byte(uint8_t data);

/* Transmits a string through UART. */
void uart_transmit_string(const char *string);

/* Extract a digit from a three-digit integer.
 *
 * position:
 * 0 = hundreds
 * 1 = tens
 * 2 = ones
 */
uint8_t extract_digit(uint16_t number, uint8_t position);

/* Convert a numerical digit from 0-9 into its ASCII character. */
uint8_t digit_to_ascii(uint8_t digit);

#endif /* UART_H_ */