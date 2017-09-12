#ifndef _UART_SETUP_H_
#define _UART_SETUP_H_

#include <inttypes.h>

typedef struct _uart_spd_setup_ {
	uint16_t	ubrr;
	uint8_t 	use_2x;
} UARTSpdT;


/**
 * vaůvulate the best possible values for uart setup to achieve desired
 * baudrate. Calculated values are returned in the UARTSpdT structure
 * where ubrr contains UBRR reg. value and use_ěx says if U2X has to be
 * involved 
 *
 * @param const uint16_t baud	a desired baud rate
 * 
 * @return UARTSpdT
 */	
UARTSpdT uartCalculateSpd(const uint16_t baud);

#endif
