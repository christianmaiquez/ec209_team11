#include "common.h"
#include "adc.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define ADC_VREF_MV   5059UL
#define ADC_MAX_COUNT 1023UL
#define MAX_PAIRS     40          // must be >= SAMPLE_COUNT

static uint16_t raw[2][MAX_PAIRS];            // [0] = ADC0 (voltage), [1] = ADC1 (current)
static volatile uint8_t sample_count;
static volatile uint8_t capture_done;
static volatile uint8_t target_samples;

void adc_init(void)
{
	ADMUX  = (1 << REFS0);                                  // AVCC reference, ADC0
	ADCSRA = (1 << ADEN) | (1 << ADPS1) | (1 << ADPS0);     // prescaler 8 -> 250kHz, 52us/conversion
	ADCSRB = 0;
	DIDR0  = (1 << ADC0D) | (1 << ADC1D) | (1 << ADC2D);
}

uint16_t adc_read(uint8_t chan)
{
	ADMUX = (ADMUX & 0xF0) | (chan & 0x0F);
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC)) {}
	return ADC;
}

uint16_t adc_convert_mv(uint16_t value)
{
	return (uint16_t)(((uint32_t)value * ADC_VREF_MV) / ADC_MAX_COUNT);
}

void adc_capture(uint16_t *v, uint16_t *i, uint8_t n_pairs)
{
	if (n_pairs > MAX_PAIRS) n_pairs = MAX_PAIRS;

	target_samples = 2 * n_pairs;
	sample_count = 0;
	capture_done = 0;

	ADMUX = (ADMUX & 0xF0);                       // start on ADC0
	ADCSRB = (1 << ADTS1) | (1 << ADTS0);         // auto-trigger: Timer0 compare match A

	// Timer0 CTC: 200 ticks at 2MHz = 100us
	TCCR0A = (1 << WGM01);
	OCR0A  = 199;
	TCNT0  = 0;
	TIFR0  = (1 << OCF0A);
	TIMSK0 = (1 << OCIE0A);                       // ISR clears the compare flag
	ADCSRA |= (1 << ADATE) | (1 << ADIE);
	TCCR0B = (1 << CS00);                         // start timer, no prescale

	while (!capture_done) {}

	for (uint8_t k = 0; k < n_pairs; k++) {
		v[k] = adc_convert_mv(raw[0][k]);
		i[k] = adc_convert_mv(raw[1][k]);
	}
}

// Empty: only needed so the compare-match flag is cleared for the auto-trigger
ISR(TIMER0_COMPA_vect) {}

ISR(ADC_vect)
{
	uint8_t n = sample_count;

	raw[n & 1][n >> 1] = ADC;                     // even = ADC0, odd = ADC1
	n++;
	sample_count = n;

	if (n >= target_samples) {
		TCCR0B = 0;                               // stop timer
		TIMSK0 = 0;
		ADCSRA &= ~((1 << ADATE) | (1 << ADIE));
		capture_done = 1;
		return;
	}
	ADMUX = (ADMUX & 0xF0) | (n & 1);             // channel for the NEXT trigger
}