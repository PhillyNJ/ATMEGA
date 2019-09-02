/*
 * hal_atmega328_timer.c
 *
 * Created: 8/18/2019 8:44:55 AM
 *  Author: pvallone
 */ 
#include <util/delay.h>
#include "atca_hal.h"

void atca_delay_ms(uint32_t delay){
	while(delay--) {
		_delay_ms(1);
	}
}

void atca_delay_us(uint32_t delay){
	while(delay--) {
		_delay_us(1);
	}
}
void atca_delay_10us(uint32_t delay){
	double d = delay * 10;
	while(d--) {
		_delay_us(1);
	}
}