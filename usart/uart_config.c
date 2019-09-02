/*
 * uart_config.c
 *
 * Created: 3/30/2017 7:23:03 AM
 *  Author: pvallone
 */ 
 #include "uart_config.h"

 static FILE uart_output = FDEV_SETUP_STREAM(usart0_putchar, NULL, _FDEV_SETUP_WRITE);
 static FILE uart_input = FDEV_SETUP_STREAM(NULL, usart0_getchar, _FDEV_SETUP_READ);

 void usart_init()
 {
	 UCSR0B |= (1 << RXEN0) | (1 << TXEN0);		//turn on rx and tx
	 UCSR0C |= (1 << UCSZ01) | (1 << UCSZ01);	//set data bits to 8

	 UBRR0H = (BAUD_PRESCALE >> 8);				//load upper 8 bits of baud value into high byte of UBBR
	 UBRR0L = (BAUD_PRESCALE);					//load lower 8 bits of baud value into high byte of UBBR
	 UCSR0B |= (1 << RXCIE0); 					// Enable the USART Receive Complete interrupt (USART_RXC)
	 stdout = &uart_output;
	 stdin  = &uart_input;
	 
 }
 
 
 void printc(char c){

	 while ((UCSR0A & (1 << UDRE0)) == 0) {};
	 UDR0 = c;

 }
 

 /* printf*/
 int usart0_putchar(char c, FILE *stream) {
	 if (c == '\n') {
		 usart0_putchar('\r', stream);
	 }
	 while ((UCSR0A & (1 << UDRE0)) == 0) {};
	 UDR0 = c;
	 return 0;
 }

 int usart0_getchar(FILE *stream) {
	 loop_until_bit_is_set(UCSR0A, RXC0);
	 return UDR0;
 }

 int getBufferchar(){
	 int retval = -1;
	 char c = -1;
	 
	 if (ring_buffer_get(_rbd, &c) == 0) {
		 retval = (int) c;
	 }

	 return retval;
 }
 
 void print_my_buffer(uint8_t *buff, uint8_t size){
	 
	 for(uint8_t i = 0; i < size;i++){
		 printf("0x%02X ", buff[i]);
	 }
	 printf("\n\r");
 }