/*
 * uart.c
 *
 * Implements the UART functions used by the project.
 * - Initialise the ATmega328P USART0 peripheral.
 * - Transmit a single byte through UART.
 * - Transmit a null-terminated string through UART.
 * - Extract individual digits from a three-digit number.
 * - Convert numerical digits from 0-9 into ASCII characters.
 *
 * Author: Team 11
 */

#define F_CPU 2000000UL

#include <avr/io.h>
#include <stdint.h>
#include "uart.h"

/*
 * UART specifications:
 *
 * CPU clock = 2 MHz
 * Baud rate = 9600
 * Data bits = 8
 * Parity    = None
 * Stop bits = 1
 *
 * UBRR = F_CPU / (16 * Baud) - 1
 *      = 2000000 / (16 * 9600) - 1
 *      = approximately 12
 */
#define UART_UBRR 12


void uart_init(void)
{
    /* Use normal-speed UART mode. */
    UCSR0A = 0;

    /* Set UART baud rate to 9600 baud. */
    UBRR0H = (uint8_t)(UART_UBRR >> 8);
    UBRR0L = (uint8_t)UART_UBRR;

    /* Enable the UART transmitter. */
    UCSR0B = (1 << TXEN0);

    /* Set frame format 8N1 */
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


void uart_transmit_byte(uint8_t data)
{
    /*
     * Wait until the UART data register is ready
     * to accept another byte.
     */
    while (!(UCSR0A & (1 << UDRE0)))
    {
        /* Wait here until UDRE0 becomes 1. */
    }

    /* Load the byte into the UART data register. */
    UDR0 = data;
}


void uart_transmit_string(const char *string)
{
    /*
     * Transmit each character until the null
     * character marking the end of the string.
     */
    while (*string != '\0')
    {
        uart_transmit_byte(*string);
        string++;
    }
}


uint8_t extract_digit(uint16_t number, uint8_t position)
{
    /* 
	 * Extracts the required digit from a
     * three-digit integer.
	 */

    switch (position)
    {
        case 0:
            return (number / 100) % 10;    /* Hundreds */

        case 1:
            return (number / 10) % 10;     /* Tens */

        case 2:
            return number % 10;            /* Ones */

        default:
            return 0;
    }
}

/* This function converts the digit to a character by adding 0 in ASCII*/
uint8_t digit_to_ascii(uint8_t digit)
{
    return digit + '0';
}