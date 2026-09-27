#include "adc.h"
#include <avr/io.h>

void adc_init(void)
{
	// AVCC reference, right-adjusted, ADC2 selected
	ADMUX = (1 << REFS0) | (1 << MUX1);

	// Enable ADC, prescaler = 16
	ADCSRA = (1 << ADEN) | (1 << ADPS2);

	// No auto trigger
	ADCSRB = 0;
}

uint16_t adc_read(uint8_t chan)
{
	uint16_t result;

	// Select ADC channel
	// Clear MUX bits
	ADMUX &= ~((1 << MUX2) | (1 << MUX1) | (1 << MUX0));
	// Select ADC channel
	if (chan == 0){
		// ADC0: 000
		}else if (chan == 1){
		// ADC1: 001
		ADMUX |= (1 << MUX0);
		}else if (chan == 2){
		// ADC2: 010
		ADMUX |= (1 << MUX1);
	}

	// Start ADC conversion
	ADCSRA |= (1 << ADSC);

	// Wait for conversion to complete
	while (ADCSRA & (1 << ADSC))
	{
	}

	// Read ADC result
	result = ADCL;
	result |= ((uint16_t)ADCH << 8);

	return result;
}

uint16_t adc_convert_mv(uint16_t value){
	return ((uint32_t)value * 5000) / 1023;
} 
