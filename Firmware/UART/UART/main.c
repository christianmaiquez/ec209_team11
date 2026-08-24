/*
 * main.c
 *
 *
 * Description:
 * Main application file for the power measurement project.
 * The program stores the RMS voltage, peak current, and power
 * values and transmits them through UART once every second.
 *
 * Author: Team 11
 */

#define F_CPU 2000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "uart.h"

#define RMSVoltage      14.5f
#define PeakCurrent     125
#define Power           1.60f

void print_rms_voltage(float voltage)
{
    uint16_t value;

    /* Remove the decimal point by multiplying by 10. */
    value = (uint16_t)((voltage * 10.0f) + 0.5f);

    /* Print hundreds and tens digits. */
    uart_transmit_byte(
        digit_to_ascii(extract_digit(value, 0))
    );

    uart_transmit_byte(
        digit_to_ascii(extract_digit(value, 1))
    );

    /* Decimal point has a fixed position. */
    uart_transmit_byte('.');

    /* Print final digit. */
    uart_transmit_byte(
        digit_to_ascii(extract_digit(value, 2))
    );
}

void print_peak_current(uint16_t current)
{
    uart_transmit_byte(
        digit_to_ascii(extract_digit(current, 0))
    );

    uart_transmit_byte(
        digit_to_ascii(extract_digit(current, 1))
    );

    uart_transmit_byte(
        digit_to_ascii(extract_digit(current, 2))
    );
}

void print_power(float power)
{
    uint16_t value;

    /* Remove the decimal point by multiplying by 100. */
    value = (uint16_t)((power * 100.0f) + 0.5f);

    /* First digit. */
    uart_transmit_byte(
        digit_to_ascii(extract_digit(value, 0))
    );

    /* Decimal point has a fixed position. */
    uart_transmit_byte('.');

    /* Second digit. */
    uart_transmit_byte(
        digit_to_ascii(extract_digit(value, 1))
    );

    /* Third digit. */
    uart_transmit_byte(
        digit_to_ascii(extract_digit(value, 2))
    );
}


int main(void)
{
    /* Configure UART before transmitting any data. */
    uart_init();

    while (1)
    {
        /* Print RMS voltage. */
        uart_transmit_string("RMS Voltage is: ");
        print_rms_voltage(RMSVoltage);
        uart_transmit_string("\r\n");

        /* Print peak current. */
        uart_transmit_string("Peak Current is: ");
        print_peak_current(PeakCurrent);
        uart_transmit_string("\r\n");

        /* Print power. */
        uart_transmit_string("Power is: ");
        print_power(Power);
        uart_transmit_string("\r\n\r\n");
		
        _delay_ms(1000);
    }

    return 0;
}