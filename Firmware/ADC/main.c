#include "adc.h"
#include <avr/io.h>

int main(void){
	adc_init(); 
	
	while(1){
		uint16_t raw = adc_read(2); 
		uint16_t mv = adc_convert_mv(raw);
	}
}
