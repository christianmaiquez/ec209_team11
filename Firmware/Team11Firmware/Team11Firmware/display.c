#include "common.h"
#include "display.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "calculations.h"

// Shift register pins (from the Proteus schematic)
#define SH_PORT PORTC
#define SH_DDR  DDRC
#define SH_CP   PC3
#define SH_DS   PC4
#define SH_ST   PC5

// Ds1 (leftmost) to Ds4 (rightmost)
static volatile uint8_t * const ds_port[4] = { &PORTD, &PORTD, &PORTD, &PORTD };
static volatile uint8_t * const ds_ddr[4]  = { &DDRD,  &DDRD,  &DDRD,  &DDRD  };
static const uint8_t ds_bit[4]             = { PD4,    PD5,    PD6,    PD7    };

#define SEG_DP 0x80

// dp = MSB, a = LSB
static const uint8_t seg_pattern[10] = {
	0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

static volatile uint8_t disp_characters[4] = {0, 0, 0, 0};   // [0] = Ds1 (left)
static volatile uint8_t disp_position = 0;

void init_display(void)
{
	SH_DDR  |= (1 << SH_CP) | (1 << SH_DS) | (1 << SH_ST);
	SH_PORT &= ~((1 << SH_CP) | (1 << SH_DS) | (1 << SH_ST));

	for (uint8_t d = 0; d < 4; d++) {
		*ds_ddr[d]  |= (1 << ds_bit[d]);
		*ds_port[d] |= (1 << ds_bit[d]);          // 1 = digit off
	}

	// Timer1 CTC, prescaler 8: 2MHz / 8 = 250kHz, 2500 ticks = 10ms
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS11);
	OCR1A  = 999;
	TIMSK1 = (1 << OCIE1A);
}

void display_show_number(float value, uint8_t unit_pattern)
{
	uint16_t n;
	uint8_t dp;                                   // which digit gets the dp (3 = none)

	if (value < 0.0f) value = 0.0f;               // no minus sign available

	if (value < 9.995f) {
		n = (uint16_t)(value * 100.0f + 0.5f);  dp = 0;     // x.xx
		} else if (value < 99.95f) {
		n = (uint16_t)(value * 10.0f + 0.5f);   dp = 1;     // xx.x
		} else {
		n = (uint16_t)(value + 0.5f);           dp = 3;     // xxx
		if (n > 999) n = 999;
	}

	uint8_t p[4];
	p[0] = seg_pattern[n / 100];
	p[1] = seg_pattern[(n / 10) % 10];
	p[2] = seg_pattern[n % 10];
	p[3] = unit_pattern;
	if (dp < 3) p[dp] |= SEG_DP;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		for (uint8_t k = 0; k < 4; k++) disp_characters[k] = p[k];
	}
}

void send_next_character_to_display(void)
{
	uint8_t pos = disp_position;
	uint8_t pattern = disp_characters[pos];

	for (int8_t b = 7; b >= 0; b--) {             // shift in, MSB first
		if (pattern & (1 << b)) SH_PORT |=  (1 << SH_DS);
		else                    SH_PORT &= ~(1 << SH_DS);
		SH_PORT |=  (1 << SH_CP);
		SH_PORT &= ~(1 << SH_CP);
	}

	for (uint8_t d = 0; d < 4; d++)               // disable all digits
	*ds_port[d] |= (1 << ds_bit[d]);

	SH_PORT |=  (1 << SH_ST);                     // latch
	SH_PORT &= ~(1 << SH_ST);

	*ds_port[pos] &= ~(1 << ds_bit[pos]);         // enable this digit (0 = on)

	disp_position = (pos + 1) & 3;
}

typedef enum { SHOW_VOLTAGE, SHOW_CURRENT, SHOW_POWER, SHOW_COUNT } quantity_t;
static quantity_t next_quantity = SHOW_VOLTAGE;

void display_show_next(void)
{
	switch (next_quantity) {
		case SHOW_VOLTAGE:  display_show_number(measure_vrms_v(),  SEG_U); break;
		case SHOW_CURRENT:  display_show_number(measure_ipk_a(),   SEG_A); break;
		default:            display_show_number(measure_power_w(), SEG_P); break;
	}
	next_quantity = (next_quantity + 1) % SHOW_COUNT;
}

// NOBLOCK: lets the ADC ISR preempt this one so sample timing stays accurate
ISR(TIMER1_COMPA_vect, ISR_NOBLOCK)
{
	send_next_character_to_display();
}