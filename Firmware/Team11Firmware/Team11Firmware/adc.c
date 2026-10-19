#include "common.h"
#include "adc.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define ADC_VREF_MV   5059UL    // measured AVCC
#define ADC_MAX_COUNT 1023UL	// Max count of the 10 bit ADC

static uint16_t *volatile buf[2];                     // where the ISR stores: [0] = voltage, [1] = current
static volatile uint8_t sample_count, target_samples;

// This function sets up the ADC
void adc_init(void)
{
	ADMUX  = (1 << REFS0);                                 // AVCC reference, channel ADC0
	ADCSRA = (1 << ADEN) | (1 << ADPS1) | (1 << ADPS0);
	ADCSRB = (1 << ADTS1) | (1 << ADTS0);                  // auto-trigger: Timer0 compare match A
	DIDR0  = (1 << ADC0D) | (1 << ADC1D) | (1 << ADC2D);   // digital input buffers off
}

// This function takes 40 pair samples of v (ADC0) and i (ADC1) alternately, 100us apart,
// and returns them in mV. Timer0 triggers every conversion, so the timing doesn't depend
// on the code; the ADC interrupt only stores the results. 
void adc_capture(uint16_t *v, uint16_t *i, uint8_t n_pairs)
{
	buf[0] = v;
	buf[1] = i;
	target_samples = 2 * n_pairs;
	sample_count = 0;

	// Timer0 CTC: 200 ticks at 2MHz = 100us between conversions
	TCCR0A = (1 << WGM01);
	OCR0A  = 199;
	TCNT0  = 0;
	TIFR0  = (1 << OCF0A);                          // clear any old compare flag
	TIMSK0 = (1 << OCIE0A);
	ADCSRA |= (1 << ADATE) | (1 << ADIE);
	TCCR0B = (1 << CS00);                           // start timer, no prescaler

	while (sample_count < target_samples) {}

	for (uint8_t k = 0; k < n_pairs; k++) {         // raw counts -> mV, in place
		v[k] = (uint32_t)v[k] * ADC_VREF_MV / ADC_MAX_COUNT;
		i[k] = (uint32_t)i[k] * ADC_VREF_MV / ADC_MAX_COUNT;
	}
}

// Empty on purpose: running it clears the compare flag, which the auto-trigger needs.
ISR(TIMER0_COMPA_vect) {}

// This ISR stores each finished conversion, selects the channel
// for the next one, and stops the timer and ADC after the last sample.
ISR(ADC_vect)
{
	uint8_t n = sample_count;

	buf[n & 1][n >> 1] = ADC;
	n++;
	ADMUX = (ADMUX & 0xF0) | (n & 1);               // next channel (ends on ADC0, ready for the next capture)

	if (n >= target_samples) {
		TCCR0B = 0;                                 // stop timer
		TIMSK0 = 0;
		ADCSRA &= ~((1 << ADATE) | (1 << ADIE));
	}
	sample_count = n;                               // last, because adc_capture() waits on this
}