#include <avr/io.h>
#include <stdio.h>


// #define BAUD_TOL 4 

#ifndef BAUD
#define BAUD 115200
#endif
#include <util/setbaud.h>


static int vPutCharUART(char c, FILE *stream) {
    if (c == '\n') {
        vPutCharUART('\r', stream);
    }
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    return 0;
}

static int vGetCharUART(FILE *stream) {
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

FILE xOutputUART = FDEV_SETUP_STREAM(vPutCharUART, NULL, _FDEV_SETUP_WRITE);
FILE xInputUART = FDEV_SETUP_STREAM(NULL, vGetCharUART, _FDEV_SETUP_READ);

void vInitUART(void) {
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    
#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */ 
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */

    stdout = &xOutputUART;
    stdin  = &xInputUART;
}
