

#include <asf.h>
#include <stdint.h>
#include "swi_bitbang_atmega328.h"
#include "atca_command.h"
#include <stdio.h>
#ifndef SWI_PIN
	#define SWI_PIN PORTB0
#endif

#define _NOP() do { __asm__ __volatile__ ("nop"); } while (0)
#define  SWI_LOW PORTB &= ~(1 << SWI_PIN); // Pin n goes low
#define  SWI_HIGH PORTB |= (1 << SWI_PIN); // Pin n goes high

SWIBuses swi_buses_default = {
    { PORTB0 }
};

//! declaration of the variable indicating which pin the selected device is connected to
static uint8_t device_pin;

void swi_set_pin(uint8_t id)
{
    device_pin = id; // not used
}

void swi_enable(void)
{
    DDRB |=  (1 << SWI_PIN);
	
}

void swi_disable(void)
{  
   DDRB &= ~(1<<SWI_PIN); //input
}


void swi_set_signal_pin(uint8_t is_high)
{
    if (is_high == 1)
    {
        SWI_HIGH; // high 	
    }
    else
    {
		SWI_LOW;      
    }
}

void swi_send_wake_token(void)
{
    swi_set_signal_pin(0);
    atca_delay_us(60);
    swi_set_signal_pin(1);
}

void swi_send_bytes(uint8_t count, uint8_t *buffer)
{
    uint8_t i, bit_mask;
    DDRB |=  (1 << SWI_PIN);
	
    cpu_irq_disable();	
	
    for (i = 0; i < count; i++)
    {
        for (bit_mask = 1; bit_mask > 0; bit_mask <<= 1)
        {
            if (bit_mask & buffer[i])   //!< Send Logic 1 (7F)
            {                
				
				SWI_LOW;                
				BIT_DELAY_1L;
                SWI_HIGH; 
                BIT_DELAY_7;
            }
            else     //!< Send Logic 0 (7D)
            {
                SWI_LOW;      
                BIT_DELAY_1L;
                SWI_HIGH;
                BIT_DELAY_1H;
                SWI_LOW;      
                BIT_DELAY_1L;
                SWI_HIGH;
                BIT_DELAY_5;
            }
        }
    }
    cpu_irq_enable();
}


void swi_send_byte(uint8_t byte)
{
    swi_send_bytes(1, &byte);
}

ATCA_STATUS swi_receive_bytes(uint8_t count, uint8_t *buffer)
{
    ATCA_STATUS status = ATCA_SUCCESS;
    uint8_t i;
    uint8_t bit_mask;
    uint8_t pulse_count;
    uint16_t timeout_count;	
	DDRB &= ~(1<<SWI_PIN); // set to receive
	
    cpu_irq_disable();
    //! Receive bits and store in buffer.
    for (i = 0; i < count; i++)
    {
        buffer[i] = 0;
        for (bit_mask = 1; bit_mask > 0; bit_mask <<= 1)
        {
            pulse_count = 0;
			
            timeout_count = START_PULSE_TIME_OUT;
            //! Detect start bit.

            while (--timeout_count > 0)
            {
                //! Wait for falling edge.
                if ((PINB & (1<<SWI_PIN)) == 0)
                {	
					//_NOP();		//pvallone		
                    break;
                }
            }
            if (timeout_count == 0)
            {
                status = ATCA_RX_TIMEOUT;
                break;
            }

            timeout_count = START_PULSE_TIME_OUT;

            do
            {
                //! Wait for rising edge. 
                if ((PINB & (1<<SWI_PIN)) != 0)
                {			
							
                    pulse_count = 1;
					
                    break;
                }
            }
            while (--timeout_count > 0);

            if (pulse_count == 0)
            {
                status = ATCA_RX_TIMEOUT;
                break;
            }

            //!  let's just wait the maximum time for the falling edge of a zero bit
            //! to arrive after we have detected the rising edge of the start bit.
            timeout_count = ZERO_PULSE_TIME_OUT;

            //! Detect possible edge indicating zero bit.
            do
            {
                if ((PINB & (1<<SWI_PIN)) == 0)
                {					
                    pulse_count = 2;
                    break;
                }
            }
            while (--timeout_count > 0);


            //! Wait for rising edge of zero pulse before returning. Otherwise we might interpret
            //! its rising edge as the next start pulse.
            if (pulse_count == 2)
            {
                timeout_count = ZERO_PULSE_TIME_OUT;

                do
                {					
                    if ((PINB & (1<<SWI_PIN)) != 0)
                    {						
                        break;
                    }
                }

                while (timeout_count-- > 0);

            }
            //! Update byte at current buffer index.
            else
            //! received "one" bit
            {
                buffer[i] |= bit_mask;
            }
        }

        if (status != ATCA_SUCCESS)
        {
            break;
        }

        if (i == 0)
        {
            if (buffer[0] < ATCA_RSP_SIZE_MIN)
            {
                status = ATCA_INVALID_SIZE;
                break;
            }
            else if (buffer[0] > count)
            {
                status = ATCA_SMALL_BUFFER;
                break;
            }
            else
            {
                count = buffer[0];
            }
        }
    }
    cpu_irq_enable();
    RX_TX_DELAY;        //forcing tTURNAROUND (To CryptoAuthentication)
    if (status == ATCA_RX_TIMEOUT)
    {
        if (i > 0)
        {
            //! Indicate that we timed out after having received at least one byte.
            status = ATCA_RX_FAIL;
        }
    }

    return status;
}