/*
 * uart_config.h
 *
 * Created: 3/30/2017 7:22:53 AM
 *  Author: pvallone
 */ 
#include <avr/io.h>
#include <stdio.h>
#include "ring_buffer.h"
#ifndef UART_CONFIG_H_
#define UART_CONFIG_H_
#define BUFFERSIZE 128 // must be in multiples of 2 
// ring buffer
static rbd_t _rbd;
static char _rbmem[BUFFERSIZE];

#ifndef BAUD_RATE
#define BAUD_RATE 9600
#endif

#ifndef F_CPU
 #define F_CPU 16000000ul
#endif

#define BAUD_PRESCALE (((F_CPU/(BAUD_RATE*16UL)))-1)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))



 /* printf*/
int usart0_putchar(char c, FILE *stream);
int usart0_getchar(FILE *stream);

void printc(char c);
int getBufferchar(void);

void usart_init(void);
void print_my_buffer(uint8_t *buff, uint8_t size);


#endif /* UART_CONFIG_H_ */