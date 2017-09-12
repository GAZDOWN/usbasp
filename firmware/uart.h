/* Author: Bc. Marcel Gazdik
 * E-Mail: gazdikmarcel (at) atlas (dot) cz
 */

#ifndef UART_H
#define UART_H

#include <inttypes.h>
#include "setup.h"

//there can be timing problem on host, so output buffer has to be large
//if we want have all bytes during wait
#define OUT_BUFFER_LEN	128
//input buffer can be so small, because keyboard input is slow
#define IN_BUFFER_LEN	4

//output  buffer struct (programmer -> host)
typedef struct buffer {
	uint8_t pos; 		//last character position
	char data[OUT_BUFFER_LEN];	
}OutBuffer;

//input buffer structure (host -> programmer)
typedef struct ibuffer {
	uint8_t len;
	uint8_t pos;
	char data[IN_BUFFER_LEN];
}InBuffer;

/*
 * Init UART with I/O buffers
 * @param baudSelector	- select real baudspeed according to which the UBRR value is computed on the fly..
 */
void uartInit(uint8_t baudSelector);
void uartDisable(void);

const OutBuffer * getRXData(void);

/*
 * This function will fill internal TX buffer with data
 * @param data - new data
 * @param len - new data length
 */
void setTXData(const unsigned char *data, uint8_t len);

//polling function (nonblocking)
void uartRXPoll(void);
void uartTXPoll(void);

#endif
