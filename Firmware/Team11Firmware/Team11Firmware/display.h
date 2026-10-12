#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

// Unit characters for the values displays (Shown on the right hand most display)
#define SEG_U 0x3E   // voltage
#define SEG_A 0x77   // current
#define SEG_P 0x73   // power

// Initialises the display
void init_display(void);

// Shows a value on Ds1-Ds3 and the corresponding unit on Ds4
void display_show_number(float value, uint8_t unit_pattern);

// Sends the next character through an ISR timer
void send_next_character_to_display(void);   

#endif