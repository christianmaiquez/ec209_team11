#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

// Unit characters for the rightmost digit (dp=MSB, a=LSB)
#define SEG_U 0x3E   // voltage
#define SEG_A 0x77   // current
#define SEG_P 0x73   // power
#define SEG_E 0x79   // energy

void init_display(void);

// Shows a value (e.g. 1.23, 12.3, 123) on Ds1-Ds3 and unit_pattern on Ds4
void display_show_number(float value, uint8_t unit_pattern);

void send_next_character_to_display(void);   // called from the Timer1 ISR

#endif