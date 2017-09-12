#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>

#ifndef F_CPU
#define F_CPU 12000000L
#endif


#define WBUF_A		0
#define WBUF_B		1

const uint16_t baudrates[] PROGMEM = {2400, 4800, 9600, 19200, 28800, 38400, 57600};

//programmer <- host
static OutBuffer a;
static OutBuffer b;

//host -> programmer
static InBuffer h_input;

static OutBuffer *current = 0;

//////////////////////////////////////////////

inline void resetBuffer(OutBuffer * buf){
	buf->pos = 0;
}

void DBufferInit(void){
	resetBuffer(&a);	
	resetBuffer(&b);	

	current = &a;
}


//implements double buffer
void uartInit(uint8_t baudSelector){
	uint16_t baudrate = pgm_read_word(&baudrates[baudSelector]);

	//clear buffers
	DBufferInit();
        UARTSpdT spd = uartCalculateSpd(baudrate); 

	//set baudrate
	//UBRRH = ((F_CPU / baudrate / 16) - 1) >> 8;
	//UBRRL = (F_CPU / baudrate / 16) - 1;
	UBRRH = spd.ubrr >> 8;
	UBRRL = spd.ubrr;
	
	if(spd.use_2x)
        	UCSRA = (1 << U2X);
	else
                UCSRA = 0x00;
 
	UCSRB = (1 << TXEN) | (1 << RXEN);
	UCSRC = (1 << UCSZ1) | (1 << UCSZ0) | (1 << URSEL);
}

void uartDisable(void){
	UCSRA = UCSRB = UCSRC = 0x00;
	current = 0;
}

const OutBuffer * getRXData(void){
	//return data buffer and switch buffers for wait
	if(current == &a){
		//reset new target buffer
		resetBuffer(&b);
		//switch buffers
		current = &b;
		//return buffer for read
		return &a;
	} else {
		resetBuffer(&a);
		current = &a;
		return &b;
	}
}

void setTXData(const unsigned char *data, uint8_t len){
	//set current data len
	h_input.len = len;
	//reset position
	h_input.pos = 0;
	
	//copy data char by char (for this moment max 3 chars)
	for(uint8_t i = 0; i < len; i++){
		h_input.data[i] = data[i];
	}
}


inline void uartRXPoll(void){
	if(current && (UCSRA & (1 << RXC)) && current->pos < OUT_BUFFER_LEN-1){
		current->data[current->pos++] = UDR;
	}
}

inline void uartTXPoll(void){
	if(UCSRA & (1 << UDRE) && h_input.pos < h_input.len){
		UDR = h_input.data[h_input.pos++];
	}
}
