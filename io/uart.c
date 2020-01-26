#include <avr/io.h>
#include <stdio.h>


// #define BAUD_TOL 4 

#ifndef BAUD
#define BAUD 115200
#endif
#include <util/setbaud.h>


static int uart_putchar(char c, FILE *stream) {
    if (c == '\n') {
        uart_putchar('\r', stream);
    }
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    return 0;
}

static int uart_getchar(FILE *stream) {
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

void uart_init(void) {
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    
#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */ 
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */






    // UBRR0 = (uint16_t)((F_CPU + BAUD * 4UL) / (BAUD * 8UL) - 1);  // for 2x mode, using 16 bit avr-gcc capability.
    // UCSR0A = _BV(U2X0); // 2x mode.     // 2x speed mode bit



    //  Enable the Rx and Tx. Also enable the Rx interrupt. The Tx interrupt will get enabled later. 
    // UCSR0B = ( _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0) );

    // /* Set the data bit register to 8n2. Two Stop Bits only affects transmitter.*/
    // UCSR0C = ( _BV(USBS0) | _BV(UCSZ01) | _BV(UCSZ00) );












    stdout = &uart_output;
    stdin  = &uart_input;
}
