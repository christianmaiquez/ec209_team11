#include "common.h"
#include "adc.h"
#include "uart.h"      // your Lab 2 UART code
#include <avr/io.h>
#include <avr/interrupt.h>

#define ADC_VREF_MV         5000UL   // AVCC = 5V
#define ADC_MAX_COUNT       1023UL   // 10-bit result
#define SAMPLES_PER_CHANNEL 40

// Capture state, shared between the ISR and the functions below (private to this file)
static uint16_t samples[2][SAMPLES_PER_CHANNEL];   // [0] = ADC0, [1] = ADC1
static volatile uint8_t sample_count;              // written by ISR
static volatile uint8_t capture_done;              // written by ISR

void adc_init(void)
{
	// ADMUX: REFS1:0 = 01 (AVCC), ADLAR = 0, MUX3:0 = 0010 (ADC2)
	ADMUX = (1 << REFS0) | (1 << MUX1);

	// ADCSRA: ADEN = 1, ADATE = 0 (single conversion), ADIE = 0 for now,
	//         ADPS2:0 = 100 -> prescaler 16 -> 2MHz / 16 = 125kHz
	ADCSRA = (1 << ADEN) | (1 << ADPS2);

	ADCSRB = 0;

	// Disable digital input buffers on ADC0-2 (saves power, less noise)
	DIDR0 = (1 << ADC0D) | (1 << ADC1D) | (1 << ADC2D);
}

uint16_t adc_read(uint8_t chan)
{
	ADMUX = (ADMUX & 0xF0) | (chan & 0x0F);   // change channel only
	ADCSRA |= (1 << ADSC);                    // start conversion
	while (ADCSRA & (1 << ADSC)) {}           // wait for it to finish
	return ADC;
}

uint16_t adc_convert_mv(uint16_t value)
{
	return (uint16_t)(((uint32_t)value * ADC_VREF_MV) / ADC_MAX_COUNT);
}

void adc_capture_and_print(void)
{
	// Start an alternating ADC0/ADC1 capture; the ISR does the rest
	sample_count = 0;
	capture_done = 0;
	ADMUX &= 0xF0;                            // start on ADC0
	ADCSRA |= (1 << ADIE) | (1 << ADSC);      // enable ADC interrupt, start first conversion

	while (!capture_done) {}                  // CPU could do other work here

	// Print two comma-separated columns (ADC0, ADC1) in mV for Excel
	for (uint8_t i = 0; i < SAMPLES_PER_CHANNEL; i++) {
		uart_put_uint(adc_convert_mv(samples[0][i]));   // swap for your own print function
		uart_puts(", ");
		uart_put_uint(adc_convert_mv(samples[1][i]));
		uart_puts("\r\n");
	}
}

// Runs each time a conversion finishes. Keep it short: no UART, no maths.
ISR(ADC_vect)
{
	uint8_t n = sample_count;

	samples[n & 1][n >> 1] = ADC;             // even samples = ADC0, odd = ADC1
	n++;
	sample_count = n;

	if (n >= 2 * SAMPLES_PER_CHANNEL) {
		ADCSRA &= ~(1 << ADIE);               // stop interrupting
		capture_done = 1;
		return;
	}

	ADMUX = (ADMUX & 0xF0) | (n & 1);         // switch to the other channel
	ADCSRA |= (1 << ADSC);                    // start next conversion
}