#ifndef _UART_H
#define _UART_H

#include <stdint.h>

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_put_uint(uint16_t value);
void uart_put_uint32(uint32_t value);
void uart_put_float1(float x);      

#endif