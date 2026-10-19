#include "common.h"
#include "display.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "calculations.h"

// Defines segment shift register pins
#define SH_PORT PORTC
#define SH_DDR  DDRC
#define SH_CP   PC3     // shift clock
#define SH_DS   PC4     // serial data
#define SH_ST   PC5     // latch

// Digit select pins on PORTD
#define DIGIT_FIRST PD4
#define DIGIT_MASK  0xF0

// Sets up bit patterns for units and decimal point on display
#define SEG_DP 0x80
#define SEG_U  0x3E     // U = voltage
#define SEG_A  0x77     // A = current
#define SEG_P  0x73     // P = power

// An array of bit patterns used to light up display to a specific number
static const uint8_t seg_pattern[10] = {
	0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

static volatile uint8_t disp_characters[4];   // Keeps track of the four numbers showing

// This function sets the display pins as outputs (all low, digits off) and starts
// Timer1 to refresh one digit every 4 ms.
void display_init(void)
{
	SH_DDR |= (1 << SH_CP) | (1 << SH_DS) | (1 << SH_ST);
	DDRD   |= DIGIT_MASK;
	PORTD  |= DIGIT_MASK;

	// Timer1 CTC, prescaler 8: 2MHz / 8 = 250kHz, 1000 ticks = 4ms per digit
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS11);
	OCR1A  = 999;
	TIMSK1 = (1 << OCIE1A);
}

// This ISR refreshes one digit per call and moves on to the next, so the four
// digits cycle fast enough to look steady (multiplexing).
// NOBLOCK: lets the ADC ISR preempt this one so sample timing stays accurate.
ISR(TIMER1_COMPA_vect, ISR_NOBLOCK)
{
	static uint8_t pos = 0;                      // digit to refresh next
	uint8_t pattern = disp_characters[pos];

	for (uint8_t mask = 0x80; mask; mask >>= 1) {   // shift the pattern in, MSB first
		if (pattern & mask) SH_PORT |=  (1 << SH_DS);
		else                SH_PORT &= ~(1 << SH_DS);
		SH_PORT |=  (1 << SH_CP);
		SH_PORT &= ~(1 << SH_CP);
	}

	PORTD |= DIGIT_MASK;                         // all digits off, so nothing smears
	SH_PORT |=  (1 << SH_ST);                    // latch the new segments
	SH_PORT &= ~(1 << SH_ST);
	PORTD &= ~(1 << (DIGIT_FIRST + pos));        // turn on this digit only

	pos = (pos + 1) & 3;                         // wrap 0-1-2-3-0
}

// This helper function shows a value on Ds1-Ds3 and the unit letter on Ds4,
// in whichever format keeps 3 digits: 1.23, 12.3 or 123.
static void display_show_number(float value, uint8_t unit_pattern)
{
	uint16_t n;                                  // the three digits as one integer
	uint8_t dp = 3;                              // digit that gets the dp (3 = none)

	if (value < 0.0f)   value = 0.0f;            // no minus sign available
	if (value > 999.0f) value = 999.0f;          // three digits is the most we can show

	if (value < 9.995f)      { n = (uint16_t)(value * 100.0f + 0.5f); dp = 0; }   // x.xx
	else if (value < 99.95f) { n = (uint16_t)(value * 10.0f + 0.5f);  dp = 1; }   // xx.x
	else                       n = (uint16_t)(value + 0.5f);                      // xxx

	uint8_t p[4] = { seg_pattern[n / 100], seg_pattern[(n / 10) % 10],
	seg_pattern[n % 10],  unit_pattern };
	if (dp < 3) p[dp] |= SEG_DP;

	// Copy with interrupts off so the ISR never sees a half-updated number
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		for (uint8_t k = 0; k < 4; k++) disp_characters[k] = p[k];
	}
}

// This function shows the next quantity from the latest capture (U, then A, then P,
// then back to U). Call it once per measure_capture().
void display_show_next(void)
{
	static uint8_t next = 0;

	switch (next) {
		case 0:  display_show_number(measure_vrms_v(),  SEG_U); break;
		case 1:  display_show_number(measure_ipk_a(),   SEG_A); break;
		default: display_show_number(measure_power_w(), SEG_P); break;
	}
	next = (next + 1) % 3;
}