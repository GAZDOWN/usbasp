#include "setup.h"

/**
 * return absolute value
 *
 * @param number
 * @return number
 */
#define ABS(x)	((x) < 0 ? ((x) * -1) : (x))

/**
 * calculate UBRR value for given bad rate (F_CPU has to be set)
 *
 * @param const uint16_t baud	a desired baudrate
 * @param const uint8_t x2	if true, values are calculated for U2X use
 *
 * @return uint16_t 		UBRR value
 */
uint16_t calculateUBRR(const uint16_t baud, const uint8_t x2){
	uint32_t div = ((uint32_t)(x2 ? 8 : 16)) * baud;
	uint32_t ubrr = F_CPU / (div) - 1;
	uint32_t ubrrm = (F_CPU % (div))*10 / div;
	
	if(ubrrm > 4)
		ubrr++;

	return ubrr;
}

/**
 * calculate baud rate from given UBRR value (F_CPU has to be set)
 *
 * @param const uint16_t ubrr	a calculated UBRR value
 * @param const uint8_t x2	if true, values are calculated for U2X use
 *
 * @return uint16_t		baud rate
 */
uint16_t calculateBAUD(const uint16_t ubrr, const uint8_t x2){
	uint16_t div =  ((uint32_t)(x2 ? 8 : 16)) * (ubrr + 1);
	uint16_t brcl = F_CPU / div;
	uint16_t brclm = (F_CPU % div)*10 / div;

	if(brclm > 4)
		brcl++;

	return brcl;
}


UARTSpdT uartCalculateSpd(const uint16_t baud){
	UARTSpdT r;

	uint16_t ubrr = calculateUBRR(baud, 0);
	uint16_t brcl = calculateBAUD(ubrr, 0);

	uint16_t ubrr2x = calculateUBRR(baud, 1);
	uint16_t brcl2x = calculateBAUD(ubrr2x, 1);

	if(ABS(((brcl*100000/baud) - 100000)) < ABS(((brcl2x*100000/baud) - 100000))){
		r.ubrr = ubrr;
		r.use_2x = 0;
	}
	else { 
	      r.ubrr = ubrr2x;
	      r.use_2x = 1;
	}

	return r;
}
